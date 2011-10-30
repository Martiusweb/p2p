#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <sys/socket.h>

/* Minimum length of PONG body for network probing response */
#define PONG_MINLEN     (sizeof(struct P2P_pong_front))

/* length of each entry in Pong body */
#define PONG_ENTRYLEN   (sizeof(struct P2P_pong_entry))


/* max number of entries for a PONG response */
#define MAX_PEER_AD     5
/* TTL value for PING (heart beat) */
#define PING_TTL_HB     1

/* The front part of the Pong message body,
   which includes the entry size */
struct P2P_pong_front {
    uint16_t    entry_size;
    uint16_t    sbz;
};

/* The structure of each entry in Pong body */
struct P2P_pong_entry {
    struct in_addr ip;
    uint16_t       port;
    uint16_t       sbz;
};

#endif
