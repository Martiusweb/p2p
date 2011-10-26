#ifndef P2P_H
#define P2P_H

#include <stdint.h>
#include <sys/socket.h>

#include "proto.h"
/* Default listening port */
#define PORT_DEFAULT    8601

/* definition of protocol message type */
#define MSG_PING        0x00
#define MSG_PONG        0x01
#define MSG_BYE         0x02
#define MSG_JOIN        0x03
#define MSG_QUERY       0x80
#define MSG_QHIT        0x81

/* Protocol version */
#define P_VERSION       1
/* MAX TTL */
#define MAX_TTL         5

/* Reply code of JOIN accept */
#define JOIN_ACC        0x0200

typedef struct {
  int client_sock;
  struct sockaddr_in local_addr;
  void* user_data;
} p2p_struct_t;

/* message header */
typedef struct {
    uint8_t     version;
    uint8_t     ttl;
    uint8_t     msg_type;
    uint8_t     reserved;
    /* the listening port of the original sender */
    uint16_t    org_port;
    /* the length of message body */
    uint16_t    length;
    /* the ip address of the original sender */
    uint32_t    org_ip;
    uint32_t    msg_id;
} p2p_header_t;

typedef struct {
  p2p_header_t header;
} p2p_msg_join_req_t;

typedef struct {
  p2p_header_t header;
  uint16_t status;
} p2p_msg_join_response_t;

typedef union {
	p2p_msg_join_req_t join_req;
	p2p_msg_join_response_t join_response;
} p2p_fixed_msg_t;

/**
 * Connects to the peer host:port.
 *
 * Returns the connected socket or and error code (< 0) :
 *  -1 is an undefined socket error (use errno)
 *  -2 is an error at join acknowledgement stage
 */
int p2p_join(p2p_struct_t* p2p, char* host, char* port);

/**
 * Close any connections.
 */
void p2p_close(p2p_struct_t* p2p);

/**
 * Read a message and write it in the given buffer.
 *
 * This method only works for messages of a fixed size.
 *
 * Returns the number of bytes read or a negative value in case of error.
 */
int p2p_read_message(p2p_struct_t* p2p, p2p_fixed_msg_t* message);

/**
 * Sends a query over th network. It won't read possible hits.
 *
 * The criteria is in the buffer key of size length.
 *
 * Returns 0 in case of succès, or a negative value in case of error.
 */
int p2p_query(p2p_struct_t* p2p, char* key, size_t length);

#endif
