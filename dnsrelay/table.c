#include "table.h"
#include "log.h"
#include "socket.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define HASH_TABLE_SIZE 1007

#define CHECK_MALLOC(ptr)                                                \
    if (ptr == NULL) {                                                   \
        log_fatal("malloc failed at %s, line %d\n", __func__, __LINE__); \
        exit(1);                                                         \
    }

static struct hlist_head hash_table[HASH_TABLE_SIZE];
static struct list_head lru_list;
static int max_cache_size;
static int cache_size;

void table_init(int max_cache_size_) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].first = NULL;
    }
    init_list_head(&lru_list);
    max_cache_size = max_cache_size_;
}

int table_load_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        log_error("Cannot open file %s (%s)\n", filename, strerror(errno));
        return 1;
    }
    char ip[16];
    char name[256];
    int cnt = 0;
    while (fscanf(fp, "%15s %255s", ip, name) == 2) {
        uint32_t addr;
        int result = inet_pton(AF_INET, ip, &addr);
        if (result != 1) {
            log_error("Invalid IPv4 address %s, skiped\n", ip);
            continue;
        }
        if (addr != 0) {
            rr_t rr = {.name = name_encode(name),
                       .type = 1,   // A
                       .class_ = 1, // IN
                       .ttl = 300,
                       .rdlength = 4,
                       .rdata = (char *)&addr};
            table_add_from_rr(rr);
        } else {
            question_t question = {.qname = name_encode(name),
                                   .qtype = 1,
                                   .qclass = 1};
            table_add_from_blocked_question(question);
            // 同时也屏蔽AAAA记录
            question.qtype = 28;
            table_add_from_blocked_question(question);
        }
        cnt++;
    }
    fclose(fp);
    log_info("Loaded %d records from %s\n", cnt, filename);
    return 0;
}

static void __table_add(struct table_entry *e) {
    if (e->type == ENTRY_TYPE_CACHE) {
        int swap = 0;
        if (cache_size == max_cache_size) {
            swap = 1;
            table_delete(container_of(lru_list.prev, struct table_entry, lru_node));
        }
        list_add(&e->lru_node, &lru_list);
        cache_size++;
        if (swap) {
            log_info("Swaped cache, cache size=%d\n", cache_size);
        } else {
            log_info("Added cache, cache size=%d\n", cache_size);
        }
    }
    uint32_t h = question_hash(e->q) % HASH_TABLE_SIZE;
    init_hlist_node(&e->hash_node);
    hlist_add_head(&e->hash_node, &hash_table[h]);
}

void table_add_from_rr(rr_t rr) {
    struct table_entry *e = (struct table_entry *)malloc(sizeof(struct table_entry));
    CHECK_MALLOC(e);

    int name_len = get_name_length(rr.name, 256);

    e->response_aaa_length = name_len + 10 + rr.rdlength;
    e->response_aaa = (char *)malloc(e->response_aaa_length);
    CHECK_MALLOC(e->response_aaa);

    memcpy(e->response_aaa, rr.name, name_len);
    struct RR rr_data = {
        .type = htons(rr.type),
        .class_ = htons(rr.class_),
        .ttl = htonl(rr.ttl),
        .rdlength = htons(rr.rdlength)};
    memcpy(e->response_aaa + name_len, &rr_data, sizeof(rr_data));
    memcpy(e->response_aaa + name_len + sizeof(rr_data), rr.rdata, rr.rdlength);

    char *qname = (char *)malloc(name_len);
    CHECK_MALLOC(qname);
    memcpy(qname, rr.name, name_len);
    e->q.qname = qname;
    e->q.qtype = rr.type;
    e->q.qclass = rr.class_;

    e->ancount = 1;
    e->nscount = 0;
    e->arcount = 0;

    e->ttl_offsets = (int *)malloc(sizeof(int));
    CHECK_MALLOC(e->ttl_offsets);
    e->ttl_offsets[0] = name_len + 4;
    e->rcode = 0;
    e->type = ENTRY_TYPE_STATIC;

    __table_add(e);
}

void table_add_from_response(const char *response, int len) {
    struct table_entry *e = (struct table_entry *)malloc(sizeof(struct table_entry));
    CHECK_MALLOC(e);

    struct HEADER *h = (struct HEADER *)response;
    int offset = sizeof(struct HEADER);
    int qdcount = ntohs(h->qdcount);
    if (qdcount != 1) {
        log_error("Unsupported: response qdcount is %d instead of 1, cancel caching\n", qdcount);
        free(e);
        return;
    }

    int qname_len = get_name_length(response + offset, len - offset);
    if (len < offset + qname_len + 4) {
        log_error("Malformed response: incomplete question section, cancel caching\n");
        free(e);
        return;
    }
    e->q.qname = (char *)malloc(qname_len);
    CHECK_MALLOC(e->q.qname);

    memcpy(e->q.qname, response + offset, qname_len);
    struct QUESTION question_data;
    memcpy(&question_data, response + offset + qname_len, sizeof(question_data));
    e->q.qtype = ntohs(question_data.qtype);
    e->q.qclass = ntohs(question_data.qclass);
    offset += qname_len + sizeof(question_data);

    e->response_aaa_length = len - offset;

    e->response_aaa = (char *)malloc(e->response_aaa_length);
    CHECK_MALLOC(e->response_aaa);
    memcpy(e->response_aaa, response + offset, e->response_aaa_length);

    e->ancount = ntohs(h->ancount);
    e->nscount = ntohs(h->nscount);
    e->arcount = ntohs(h->arcount);
    e->rcode = h->rcode;

    // 根据 RFC 2308，NXDOMAIN 回应的 negative cache ttl 应该是 SOA 的 ttl 和 SOA.MINIMUM 中的较小值，
    // 但在实际中，SOA 的 ttl 似乎永远不会大于 SOA.MINIMUM，所以我们只使用 SOA 的 ttl，这也简化了代码

    int ttl_cnt = e->ancount + e->nscount + e->arcount;
    e->min_ttl = ttl_cnt ? UINT_MAX : 300;
    e->ttl_offsets = (int *)malloc(sizeof(int) * ttl_cnt);
    CHECK_MALLOC(e->ttl_offsets);

    int offset_aaa = offset;

    for (int i = 0; i < ttl_cnt; i++) {
        int name_len = get_name_length(response + offset, len - offset);
        if (len < offset + name_len + 10) {
            log_error("Malformed response: incomplete answer/authority/additional section, cancel caching\n");
            free(e->response_aaa);
            free(e->q.qname);
            free(e);
            return;
        }
        offset += name_len;
        e->ttl_offsets[i] = offset + 4 - offset_aaa;
        uint32_t ttl;
        memcpy(&ttl, response + offset + 4, 4);
        ttl = ntohl(ttl);
        if (ttl < e->min_ttl) {
            e->min_ttl = ttl;
        }
        uint16_t rdlength;
        memcpy(&rdlength, response + offset + 8, 2);
        rdlength = ntohs(rdlength);
        offset += 10 + rdlength;
    }

    e->receive_time = time(0);
    e->type = ENTRY_TYPE_CACHE;

    __table_add(e);
}

void table_add_from_blocked_question(question_t question) {
    struct table_entry *e = (struct table_entry *)malloc(sizeof(struct table_entry));
    CHECK_MALLOC(e);
    int name_len = get_name_length(question.qname, 256);
    e->q.qname = malloc(name_len);
    CHECK_MALLOC(e->q.qname);
    memcpy(e->q.qname, question.qname, name_len);
    e->q.qtype = question.qtype;
    e->q.qclass = question.qclass;
    e->response_aaa = NULL;
    e->response_aaa_length = 0;
    e->ancount = 0;
    e->nscount = 0;
    e->arcount = 0;
    e->rcode = 3;
    e->ttl_offsets = NULL;
    e->receive_time = 0;
    e->type = ENTRY_TYPE_STATIC;
    __table_add(e);
}

struct table_entry *table_lookup(question_t q) {
    int h = question_hash(q) % HASH_TABLE_SIZE;
    for (struct hlist_node *node = hash_table[h].first; node; node = node->next) {
        struct table_entry *e = container_of(node, struct table_entry, hash_node);
        if (question_equal(q, e->q)) {
            if (e->type == ENTRY_TYPE_CACHE) {
                list_del(&e->lru_node);
                list_add(&e->lru_node, &lru_list);
            }
            return e;
        }
    }
    return NULL;
}

void table_delete(struct table_entry *entry) {
    if (entry->type == ENTRY_TYPE_CACHE) {
        list_del(&entry->lru_node);
        cache_size--;
    }
    hlist_del(&entry->hash_node);
    free(entry->q.qname);
    free(entry->response_aaa);
    free(entry->ttl_offsets);
    free(entry);
}
