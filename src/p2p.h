#ifndef P2P_H
#define P2P_H

#include <stdint.h>
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

/* Query hit structures */
typedef struct {
  uint16_t id;
  uint32_t value;
} p2p_qhit_resource_t;

typedef struct {
  p2p_header_t header;
  uint16_t nb_entries;
  p2p_qhit_resource_t* entries;
} p2p_qhit_t;

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
 * Read a message and write it in the given buffer.
 *
 * This method only works for messages of a fixed size.
 *
 * Returns the number of bytes read or a negative value in case of error.
 */
int p2p_read_fixed_message(p2p_struct_t* p2p, p2p_fixed_msg_t* message);

/**
 * Sends a query over th network. It won't read possible hits.
 *
 * The criteria is in the buffer key of size length.
 *
 * Returns 0 in case of succès, or a negative value in case of error.
 */
int p2p_query(p2p_struct_t* p2p, char* key, size_t length);

/*ù
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

/**
 * Reads a message into a buffer allocated to fit the message size on the socket
 * socket.
 *
 * The socket parameter must be one of the sockets created and managed by this
 * library. You can get the descriptors with the p2p_struct_t, fields
 * client_sock and server_sock.
 *
 * Endianness of the header will be updated to host endianness. Message will be
 * kept untouched.
 *
 * You should not use this method directly but use a wrapper which know which
 * socket to use.
 *
 * On error, the message pointer points to 0.
 */
int p2p_alloc_read_message(p2p_struct_t* p2p, int socket, char** message);

/**
 * Reads a query hit.
 *
 * Returns a negative value in case of error. If the message read is not the
 * expected one, this method will fail.
 *
 * Returns a negative value on error.
 *
 * An array of query resources will be allocated if any. If there is no entry,
 * the ressource array points to null.
 *
 * On error, the message pointer points to 0.
 */
int p2p_read_query_hit(p2p_struct_t* p2p, p2p_qhit_t* query_hit);

#endif
