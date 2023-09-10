#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "datalink.h"
#include "protocol.h"

#define MAX_SEQ 63
#define NR_BUFS ((MAX_SEQ + 1) / 2)
#define DATA_TIMER 2000
#define ACK_TIMER 300

typedef unsigned char buffer_t[PKT_LEN];
typedef unsigned char seq_nr;

static inline seq_nr next(seq_nr x) {
    return x == MAX_SEQ ? 0 : x + 1;
}

static inline seq_nr prev(seq_nr x) {
    return x == 0 ? MAX_SEQ : x - 1;
}

static inline int between(seq_nr begin, seq_nr t, seq_nr end) {
    return (t >= begin && t < end)
        || (begin > end && t >= begin)
        || (begin > end && t < end);
}
struct FRAME {
    unsigned char kind;
    seq_nr ack;
    seq_nr seq;
    buffer_t data;
    unsigned int padding;
};

// sending window: [sw_begin, sw_end)
static seq_nr sw_begin = 0, sw_end = 0, sw_size = 0;
static buffer_t sw_buffer[NR_BUFS];

// receiving window: [rw_begin, rw_end)
static seq_nr rw_begin = 0, rw_end = NR_BUFS;
static buffer_t rw_buffer[NR_BUFS];
static int arrived[NR_BUFS];

static int phl_ready = 0;
static int sended_nak = 0;

static void put_frame(unsigned char *frame, int len) {
    *(unsigned int *)(frame + len) = crc32(frame, len);
    send_frame(frame, len + 4);
    phl_ready = 0;
}

static void send_data_frame(seq_nr seq) {
    struct FRAME s;

    s.kind = FRAME_DATA;
    s.seq = seq;
    s.ack = prev(rw_begin);
    memcpy(s.data, sw_buffer[seq % NR_BUFS], PKT_LEN);

    dbg_frame("Send DATA %d %d, ID %d\n", s.seq, s.ack, *(short *)s.data);
    stop_ack_timer();
    put_frame((unsigned char *)&s, 3 + PKT_LEN);

    start_timer(seq, DATA_TIMER);
}

static void send_ack_frame(void) {
    struct FRAME s;

    s.kind = FRAME_ACK;
    s.ack = prev(rw_begin);

    dbg_frame("Send ACK  %d\n", s.ack);

    put_frame((unsigned char *)&s, 2);
}

static void send_sep_ack_frame(seq_nr ack) {
    struct FRAME s;

    s.kind = FRAME_SEP_ACK;
    s.ack = ack;

    dbg_frame("Send Sep ACK  %d\n", s.ack);

    put_frame((unsigned char *)&s, 2);
}

static void send_nak_frame(void) {
    sended_nak = 1;

    struct FRAME s;

    s.kind = FRAME_NAK;
    s.ack = prev(rw_begin);

    dbg_frame("Send NAK  %d\n", s.ack);
    stop_ack_timer();
    put_frame((unsigned char *)&s, 2);
}

int main(int argc, char **argv) {
    protocol_init(argc, argv);
    lprintf("Selective Repeat, Designed by Ren Fei, build: " __DATE__ "  "__TIME__"\n");
    disable_network_layer();
    for (;;) {
        int arg;
        int event = wait_for_event(&arg);
        switch (event) {
        case NETWORK_LAYER_READY:
            get_packet(sw_buffer[sw_end % NR_BUFS]);
            send_data_frame(sw_end);
            sw_end = next(sw_end);
            sw_size++;
            break;

        case PHYSICAL_LAYER_READY:
            phl_ready = 1;
            break;

        case FRAME_RECEIVED:
            struct FRAME f;
            int len = recv_frame((unsigned char *)&f, sizeof f);
            if (len < 5 || crc32((unsigned char *)&f, len) != 0) {
                if (len > 5) {
                    dbg_event("**** Receiver Error, Bad CRC Checksum\n");
                    if (!sended_nak) {
                        send_nak_frame();
                    }
                }
                break;
            }
            if (f.kind == FRAME_SEP_ACK) {
                dbg_frame("Recv Sep ACK  %d\n", f.ack);
                stop_timer(f.ack);
                break;
            }
            if (f.kind == FRAME_ACK)
                dbg_frame("Recv ACK  %d\n", f.ack);
            if (f.kind == FRAME_DATA) {
                dbg_frame("Recv DATA %d %d, ID %d\n", f.seq, f.ack, *(short *)f.data);
                if (between(rw_begin, f.seq, rw_end)) {
                    if (f.seq != rw_begin && !sended_nak) {
                        send_nak_frame();
                    }
                    if (!arrived[f.seq % NR_BUFS]) {
                        memcpy(rw_buffer[f.seq % NR_BUFS], f.data, PKT_LEN);
                        arrived[f.seq % NR_BUFS] = 1;
                        if (f.seq != rw_begin) {
                            send_sep_ack_frame(f.seq);
                        }
                        while (arrived[rw_begin % NR_BUFS]) {
                            put_packet(rw_buffer[rw_begin % NR_BUFS], len - 7);
                            arrived[rw_begin % NR_BUFS] = 0;
                            rw_begin = next(rw_begin);
                            rw_end = next(rw_end);
                            sended_nak = 0;
                            start_ack_timer(ACK_TIMER);
                        }
                    }
                } else {
                    dbg_event("Recv Useless DATA\n");
                }
            }
            while (between(sw_begin, f.ack, sw_end)) {
                stop_timer(sw_begin);
                sw_begin = next(sw_begin);
                sw_size--;
            }
            if (f.kind == FRAME_NAK) {
                dbg_frame("Recv NAK  %d\n", f.ack);
                if (between(sw_begin, next(f.ack), sw_end)) {
                    send_data_frame(next(f.ack));
                }
            }
            break;

        case DATA_TIMEOUT:
            dbg_event("---- DATA %d timeout\n", arg);
            send_data_frame(arg);
            break;

        case ACK_TIMEOUT:
            dbg_event("---- ACK timeout %d\n", prev(rw_begin));
            send_ack_frame();
            break;
        }
        if (sw_size == NR_BUFS) {
            dbg_event("**** The sending window is full. (%d)\n", sw_size);
        }
        if (sw_size < NR_BUFS && phl_ready) {
            enable_network_layer();
        } else {
            disable_network_layer();
        }
    }
}