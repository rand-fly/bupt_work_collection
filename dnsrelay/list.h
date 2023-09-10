#pragma once
#include <stddef.h>
#include <stdlib.h>

#define container_of(ptr, type, member) ((type *)((char *)(ptr)-offsetof(type, member)))

struct list_head {
    struct list_head *next, *prev;
};

static inline void init_list_head(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head) {
    __list_add(new, head, head->next);
}

static inline void list_del(struct list_head *entry) {
    struct list_head *prev = entry->prev;
    struct list_head *next = entry->next;
    prev->next = next;
    next->prev = prev;
}

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

static inline void init_hlist_node(struct hlist_node *h) {
    h->next = NULL;
    h->pprev = NULL;
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h) {
    struct hlist_node *first = h->first;
    n->next = first;
    if (first) first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

static inline void hlist_del(struct hlist_node *n) {
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next) next->pprev = pprev;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define list_entry(ptr, type, member) container_of(ptr, type, member)
