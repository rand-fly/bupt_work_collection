#include "id_convert.h"
#include "log.h"
#include "socket.h"
#include "table.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#include "getopt.h"
#define getopt_long getopt_int
#else
#include <getopt.h>
#endif

#define BUFLEN 1024

static struct option intopts[] = {
    {"help", no_argument, NULL, '?'},
    {"log", required_argument, NULL, 'l'},
    {"stat-int", required_argument, NULL, 's'},
    {"host", required_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {"remote", required_argument, NULL, 'r'},
    {"cache-size", required_argument, NULL, 'c'},
    {"file", required_argument, NULL, 'f'},
    {0, 0, 0, 0},
};

static const char *shortops = "?l:s:h:p:r:c:f:";

static const char *usage =
    "Usage: dnsrelay [options]\n"
    "Options:\n"
    "  -?, --help\t\t\tShow this help message\n"
    "  -l, --log <0-5>\t\tSet log level, default 4 (Info)\n"
    "  -s, --stat-int <interval>\tOutput statistic info every (at least) <interval> seconds\n"
    "                           \tSet to 0 to disable outputing statistic info, default 10\n"
    "  -h, --host <host>\t\tSet host address, default ::\n"
    "  -p, --port <port>\t\tSet udp port, default 53\n"
    "  -r, --remote <server>\t\tSet remote server, default 1.1.1.1\n"
    "  -c, --cache-size <size>\tSet cache size, default 1000\n"
    "  -f, --file <filename>\t\tSet static file, default dnsrelay.txt\n";

static socket_t s;
static address_t server_addr;

// 统计信息
static uint64_t query_count, cache_hit_count, cache_outdate_count;
static uint64_t remote_send_count, remote_recv_count;

void handle_query(char *buf, int len, address_t client_addr);
void handle_response(char *buf, int len);

int main(int argc, char **argv) {
    int c;
    int log_level = LOG_LEVEL_INFO;
    char *host = "::";
    int port = 53;
    char *remote_server = "1.1.1.1";
    int cache_size = 1000;
    char *static_file = "dnsrelay.txt";
    int statistic_interval = 10;
    int last_statistic_time = time(0);
    while ((c = getopt_long(argc, argv, shortops, intopts, NULL)) != -1) {
        switch (c) {
        case 'l':
            log_level = atoi(optarg);
            if (log_level < 0 || log_level > 5) {
                log_fatal("log level must be between 0 and 5\n");
                exit(1);
            }
            break;
        case 's':
            statistic_interval = atoi(optarg);
            if (statistic_interval < 0) {
                log_fatal("statistic interval must be greater than or equal 0\n");
                exit(1);
            }
            break;
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            if (port < 0 || port > 65535) {
                log_fatal("port must be between 0 and 65535\n");
                exit(1);
            }
            break;
        case 'r':
            remote_server = optarg;
            break;
        case 'c':
            cache_size = atoi(optarg);
            if (cache_size < 0 || cache_size > 1000000000) {
                log_fatal("cache size must be between 0 and 1000000000\n");
                exit(1);
            }
            break;
        case 'f':
            static_file = optarg;
            break;
        case '?':
            puts(usage);
            return 0;
        default:
            abort();
        }
    }
    int result;

    log_init();
    log_set_level(log_level);
    log_info("dnsrelay designed by Ren Fei, compiled at " __TIME__ " " __DATE__ "\n");

    result = socket_init(&s, host, port);
    if (result != 0) exit(1);

    result = resolve_address(&server_addr, remote_server, 53);
    if (result != 0) {
        log_fatal("Fail to resolve remote server\n");
        exit(1);
    }

    log_info("Using remote server %s\n", remote_server);
    log_info("Cache size: %d\n", cache_size);
    table_init(cache_size);
    table_load_file(static_file);

    for (;;) {
        address_t sender_addr;
        char buf[BUFLEN];
        int len;
        if (socket_recv(s, &sender_addr, buf, BUFLEN, &len) != 0) return 0;

        if (len <= (int)sizeof(struct HEADER)) {
            log_error("Malformed packet: incomplete header\n");
            continue;
        }

        struct HEADER *h = (struct HEADER *)buf;
        if (h->qr == 0) {
            query_count++;
            handle_query(buf, len, sender_addr);
        } else {
            int correct_host = memcmp(&sender_addr.sin6_addr, &server_addr.sin6_addr, sizeof(struct in6_addr)) == 0;
            int correct_port = sender_addr.sin6_port == server_addr.sin6_port;
            if (correct_host && correct_port) {
                remote_recv_count++;
                handle_response(buf, len);
            } else {
                log_error("Receive response from %s:%d instead of %s:%d\n",
                          address_host(sender_addr), sender_addr.sin6_port,
                          address_host(server_addr), server_addr.sin6_port);
            }
        }
        time_t now = time(0);
        if (statistic_interval && now - last_statistic_time >= statistic_interval) {
            last_statistic_time = now;
            double hit_rate = 100.0 * cache_hit_count / query_count;
            log_stat("Query: %" PRId64 ", Cache hit: %" PRId64 ", Hit rate: %.2lf%%, "
                     "Cache outdate: %" PRId64 ", Remote send: %" PRId64 ", Remote recv: %" PRId64 "\n",
                     query_count, cache_hit_count, hit_rate,
                     cache_outdate_count, remote_send_count, remote_recv_count);
        }
    }
}

void handle_query(char *buf, int len, address_t client_addr) {
    int result;

    struct HEADER *h = (struct HEADER *)buf;
    int offset = sizeof(struct HEADER);

    int qdcount = ntohs(h->qdcount);
    if (qdcount != 1) {
        // 尽管在 RFC 1035 中一个报文可以包含多个询问，但是实际上没有人这么做
        log_error("Unsupported: query qdcount is %d instead of 1, discarded\n", qdcount);
        return;
    }

    char qname_buf[256];
    int qname_len = get_name_length(buf + offset, len - offset);
    if (len < offset + qname_len + 4) {
        log_error("Malformed query: incomplete question\n");
        return;
    }
    memcpy(qname_buf, buf + offset, qname_len);

    struct QUESTION question_data;
    memcpy(&question_data, buf + offset + qname_len, sizeof(question_data));
    question_t q = {.qname = qname_buf,
                    .qtype = ntohs(question_data.qtype),
                    .qclass = ntohs(question_data.qclass)};
    offset += qname_len + sizeof(question_data);

    log_debug("Receive query from %s id=%d name=%s type=%d class=%d\n",
              address_host(client_addr), ntohs(h->id),
              name_decode(q.qname), q.qtype, q.qclass);

    struct table_entry *entry = table_lookup(q);
    time_t dtime;
    int modify_ttl = 0;

    if (entry == NULL) {
        log_info("Cache miss name=%s type=%d class=%d\n",
                 name_decode(q.qname), q.qtype, q.qclass);

    } else if (entry->type == ENTRY_TYPE_STATIC) {
        log_debug("Read from static name=%s type=%d class=%d\n",
                  name_decode(q.qname), q.qtype, q.qclass);

    } else if (entry->type == ENTRY_TYPE_CACHE) {
        dtime = time(0) - entry->receive_time;
        if (dtime < entry->min_ttl) {
            modify_ttl = 1;
            log_debug("Read from cache name=%s type=%d class=%d\n",
                      name_decode(q.qname), q.qtype, q.qclass);
        } else {
            log_info("Cache outdated name=%s type=%d class=%d\n",
                     name_decode(q.qname), q.qtype, q.qclass);
            cache_outdate_count++;
            table_delete(entry);
            entry = NULL;
        }
    }

    if (entry == NULL) {
        session_t session = {.addr = client_addr,
                             .cid = ntohs(h->id)};
        uint16_t rid = allocate_remote_id(session);
        h->id = htons(rid);
        log_debug("Send to remote, remote id=%d\n", rid);
        remote_send_count++;
        result = socket_send(s, &server_addr, buf, len);
        if (result != 0) exit(1);
    } else {
        cache_hit_count++;
        h->qr = 1;
        h->ra = 1;
        h->aa = 0;
        h->rcode = entry->rcode;
        h->qdcount = htons(1);
        h->ancount = htons(entry->ancount);
        h->nscount = htons(entry->nscount);
        h->arcount = htons(entry->arcount);
        if (entry->response_aaa_length > 0) {
            memcpy(buf + offset, entry->response_aaa, entry->response_aaa_length);
        }
        if (modify_ttl) {
            int ttl_cnt = entry->ancount + entry->nscount + entry->arcount;
            for (int i = 0; i < ttl_cnt; i++) {
                uint32_t *ttl_p = (uint32_t *)(buf + offset + entry->ttl_offsets[i]);
                uint32_t ttl;
                memcpy(&ttl, ttl_p, 4);
                ttl = htonl(ntohl(ttl) - dtime);
                memcpy(ttl_p, &ttl, 4);
            }
        }
        result = socket_send(s, &client_addr, buf, offset + entry->response_aaa_length);
        if (result != 0) exit(1);
    }
}

void handle_response(char *buf, int len) {
    struct HEADER *h = (struct HEADER *)buf;
    // 只缓存 NOERROR 和 NXDOMAIN 响应
    if (h->opcode == 0 && (h->rcode == 0 || h->rcode == 3)) {
        table_add_from_response(buf, len);
    }
    uint16_t rid = ntohs(h->id);
    session_t session = rid_to_session(rid);
    log_debug("Receive from remote, remote id=%d\n", rid);
    h->id = htons(session.cid);
    h->aa = 0;
    log_debug("Send to %s id=%d\n", address_host(session.addr), session.cid);
    int result = socket_send(s, &session.addr, buf, len);
    if (result != 0) exit(1);
}
