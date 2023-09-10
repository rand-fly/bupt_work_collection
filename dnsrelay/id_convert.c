#include "id_convert.h"

static session_t id_convert_table[65536];
static uint16_t rid_next;

uint16_t allocate_remote_id(session_t s) {
    id_convert_table[rid_next] = s;
    return rid_next++;
}

session_t rid_to_session(uint16_t rid) {
    return id_convert_table[rid];
}
