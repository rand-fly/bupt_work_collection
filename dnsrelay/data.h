#pragma once
#include <stdint.h>
#include <sys/types.h>

// 大写名称结构体仅包含报文中的定长部分，其内存布局与报文一致，使用网络字节序，用于解析和构造报文
// 小写名称结构体还包含报文中的变长部分，其内存布局不一定与报文一致，使用主机字节序，用于在程序内部传递数据

#pragma pack(1)
struct HEADER {
    unsigned id : 16;    /* query identification number */
    unsigned rd : 1;     /* recursion desired */
    unsigned tc : 1;     /* truncated message */
    unsigned aa : 1;     /* authoritive answer */
    unsigned opcode : 4; /* purpose of message */
    unsigned qr : 1;     /* response flag */
    unsigned rcode : 4;  /* response code */
    unsigned cd : 1;     /* checking disabled by resolver */
    unsigned ad : 1;     /* authentic data from named */
    unsigned z : 1;      /* unused bits, must be ZERO */
    unsigned ra : 1;     /* recursion available */
    uint16_t qdcount;    /* number of question entries */
    uint16_t ancount;    /* number of answer entries */
    uint16_t nscount;    /* number of authority entries */
    uint16_t arcount;    /* number of resource entries */
};

struct QUESTION {
    uint16_t qtype;
    uint16_t qclass;
};

struct RR {
    uint16_t type;
    uint16_t class_;
    uint32_t ttl;
    uint16_t rdlength;
};
#pragma pack()

typedef struct question {
    char *qname;
    uint16_t qtype;
    uint16_t qclass;
} question_t;

typedef struct rr {
    char *name;
    uint16_t type;
    uint16_t class_;
    uint32_t ttl;
    uint16_t rdlength;
    char *rdata;
} rr_t;

// 将域名从可读字符串编码为DNS报文中的格式
// 注意该函数的输出为一个固定缓冲区，后续调用会覆盖前一次的结果
char *name_encode(const char *s);

// 将域名从DNS报文中的格式还原为可读字符串
// 注意该函数的输出为一个固定缓冲区，后续调用会覆盖前一次的结果
char *name_decode(const char *s);

// 获取编码后的域名长度，不超过maxlen
int get_name_length(const char *s, int maxlen);

uint32_t question_hash(question_t q);

int question_equal(question_t a, question_t b);
