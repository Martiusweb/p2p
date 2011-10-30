#ifndef P2P_H
#define P2P_H

#include <netinet/in.h>
#include <sys/socket.h>

#include "proto.h"

/* Max clients that will wait in accept queue */
#define MAX_WAITING_CLIENTS (2)
/* Default listening port */
#define PORT_DEFAULT    (8601)

/* Protocol version */
#define P_VERSION       (1)
/* MAX TTL */
#define MAX_TTL         (5)
/* Reply code of JOIN accept */
#define JOIN_ACC        (0x0200)

/* definition of protocol message type */
#define MSG_PING        (0x00)
#define MSG_PONG        (0x01)
#define MSG_BYE         (0x02)
#define MSG_JOIN        (0x03)
#define MSG_QUERY       (0x80)
#define MSG_QHIT        (0x81)

typedef struct {
	/* Client socket identifier */
  int client_sock;
	/* Address bound to the client socket interface
	 * Warning : this is NOT the peer address) */
  struct sockaddr_in local_addr;
	/* Server socket identifier */
	int server_sock;
	/* Address bound to the server socket */
	struct sockaddr_in server_addr;
	/* */
  void* user_data;
} p2p_struct_t;

/* The Header definition of our Protocol */
struct P2P_h {
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
};

struct p2p_msg_join_req {
  struct P2P_h header;
};

struct p2p_msg_join_response {
  struct P2P_h header;
  uint16_t status;
};

/**
 * Initializes the internal p2p structure.
 */
void p2p_init(p2p_struct_t* p2p);

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
 * Create a listening socket on interface host and get prepared to listen on
 * port port.
 *
 * Returns the connected socket or an error code (< 0).
 */
int p2p_listen(p2p_struct_t* p2p, char* host, char* port);

/**
 * Accept an incoming connection. A listening socket must be set with
 * p2p_listen.
 *
 * Returns the peer socket or a negative value in case of error.
 */
int p2p_accept(p2p_struct_t* p2p);

#endif
