#pragma once
#include "data.h"
#include "list.h"
#include <stdint.h>

struct table_entry {
    struct hlist_node hash_node;
    struct list_head lru_node;
    question_t q;                       // 问题三元组，用于查询和构造 Question Section
    uint16_t ancount, nscount, arcount; // 用于计算需要修改的 ttl 个数和构造 Header Section
    uint8_t rcode;                      // 用于构造 Header Section
    char *response_aaa;                 // Answer/Authority/Additional Section 原文
    int response_aaa_length;            // response_aaa 长度
    int *ttl_offsets;                   // 各个ttl字段在 response_aaa 中的偏移量
    uint32_t min_ttl;                   // 最小 ttl，用于判断是否过期
    uint64_t receive_time;
    enum { ENTRY_TYPE_STATIC,
           ENTRY_TYPE_CACHE } type;
};

void table_init(int max_cache_size_);

int table_load_file(const char *filename);

void table_add_from_rr(rr_t rr);

void table_add_from_response(const char *response, int len);

void table_add_from_blocked_question(question_t question);

struct table_entry *table_lookup(question_t q);

void table_delete(struct table_entry *entry);
