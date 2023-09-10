#pragma once
#include "socket.h"
#include <stdint.h>

typedef struct session {
    uint16_t cid;
    address_t addr;
} session_t;

uint16_t allocate_remote_id(session_t s);

session_t rid_to_session(uint16_t rid);
