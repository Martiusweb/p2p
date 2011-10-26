#ifndef P2P_H
#define P2P_H

#include <sys/socket.h>

#include "proto.h"

typedef struct {
  int client_sock;
  struct sockaddr_in local_addr;
  void* user_data;
} p2p_struct_t;

typedef struct {
  struct P2P_h header;
} p2p_msg_join_req_t;

typedef struct {
  struct P2P_h header;
  uint16_t status;
} p2p_msg_join_response_t;

typedef union {
	p2p_msg_join_req_t join_req;
	p2p_msg_join_response_t join_response;
} p2p_msg_t;

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
 * Returns the number of bytes read or a negative value in case of error.
 */
int p2p_read_message(p2p_struct_t* p2p, p2p_msg_t* message);

#endif
