#include "data.h"
#include <stdlib.h>
#include <string.h>

char *name_encode(const char *s) {
    static char d[256];
    int len = (int)strlen(s);
    if (len > 254) len = 254;
    if (len > 0 && s[len - 1] == '.') len--;
    int cnt = 0;
    d[len + 1] = 0;
    for (int i = len; i >= 1; i--) {
        if (s[i - 1] == '.') {
            d[i] = (char)cnt;
            cnt = 0;
        } else {
            d[i] = (char)s[i - 1];
            cnt++;
        }
    }
    d[0] = (char)cnt;
    return d;
}

char *name_decode(const char *s) {
    static char d[256];
    int cnt = s[0];
    int i;
    for (i = 1; i < 256 && s[i]; i++) {
        if (cnt > 0) {
            d[i - 1] = s[i];
            cnt--;
        } else {
            d[i - 1] = '.';
            cnt = s[i];
        }
    }
    d[i - 1] = 0;
    return d;
}

int get_name_length(const char *s, int maxlen) {
    int len = 0;
    while (s[len]) {
        if ((s[len] & 0xc0) == 0xc0) {
            len += 2;
            if (len >= maxlen) return maxlen;
            return len;
        }
        len++;
        if (len > maxlen) return maxlen;
    }
    if (len > maxlen) return maxlen;
    return len + 1;
}

uint32_t question_hash(question_t q) {
    uint64_t h = (q.qtype << 16) + q.qclass;
    for (char *s = q.qname; *s; s++) {
        h = (h * 17 + *s) % 1000000007;
    }
    return (uint32_t)h;
}

int question_equal(question_t a, question_t b) {
    if (strcmp(a.qname, b.qname) != 0) return 0;
    if (a.qtype != b.qtype) return 0;
    if (a.qclass != b.qclass) return 0;
    return 1;
}
