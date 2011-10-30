#define DEBUG

#include "p2p.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "sfhash.h"

/**
 * Sets the message header with default values.
 */
static void p2p_set_header(p2p_struct_t* p2p, struct P2P_h* header, uint8_t msg_type) {
  struct {
    uint16_t port;
    uint32_t ip;
    time_t time;
  } msg_id_seed;

  header->version = P_VERSION;
  header->ttl = MAX_TTL;
  header->msg_type = msg_type;
  header->length = 0;

  header->org_port = p2p->local_addr.sin_port;
  header->org_ip = p2p->local_addr.sin_addr.s_addr;

  msg_id_seed.port = ntohs(header->org_port);
  msg_id_seed.ip   = ntohl(header->org_ip);
  msg_id_seed.time = time(NULL);
  header->msg_id = htonl(SuperFastHash((const char*) &msg_id_seed, sizeof(msg_id_seed)));
}

void p2p_init(p2p_struct_t* p2p) {
  memset(p2p, 0, sizeof(p2p));
}

int p2p_join(p2p_struct_t* p2p, char* host, char* port) {
  struct sockaddr_in h_addr;
  struct sockaddr_in l_addr;
  int client_sock;
  int call_state;
  int to_receive;
  struct p2p_msg_join_req join;
  struct p2p_msg_join_response join_response;
  char buffer[sizeof(struct p2p_msg_join_response)];
  char* buffer_pos;
  socklen_t l_add_len;

  memset(&h_addr, 0, sizeof(h_addr));
  /* We connect using TCP/Ipv4 */
  h_addr.sin_family = AF_INET;
  /* Parse the IP and format it for the low-level struct */
  h_addr.sin_addr.s_addr = inet_addr(host);
  /* Parse the port string in integer (atoi) and convert to network byte order
   * (htons) */
  h_addr.sin_port = htons(atoi(port));

	if(h_addr.sin_addr.s_addr == INADDR_NONE) {
		return -1;
	}

  /* Create the socket :
   *  - AF_INET (equivalent to PF_INET) : IPv4
   *  - SOCK_STREAM : a TCP STREAM
   *  - IPPROTO_TCP : tells that the protocol is TCP (even if SOCK_STREAM
   *    actualy implies it).
   **/
  if((client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    /* Failed : return socket error code */
    return client_sock;
  }

  if((call_state = connect(client_sock, (struct sockaddr*) &h_addr, sizeof(h_addr))) < 0) {
    /* Failed : return connect error code */
    close(client_sock);
    return call_state;
  }

  /* get the IP of my bound interface */
  if((call_state = getsockname(client_sock, (struct sockaddr*) &l_addr, &l_add_len)) < 0) {
    close(client_sock);
    return -1;
  }

  /* create a Join message */
  memset(&join, 0, sizeof(join));
  p2p_set_header(p2p, &join.header, MSG_JOIN);

  /* send join request */
  if(send(client_sock, &join, sizeof(join), 0) != sizeof(join)) {
    close(client_sock);
    return -1;
  }

  /* receive join acknowledgement, I except this type of message, if the message
   * is not the right one, the behavior may be undefined.
   */
  to_receive = sizeof(join_response);
  buffer_pos = buffer;
  while(to_receive > 0) {
    int bytes = 0;
    if((bytes = recv(client_sock, buffer_pos, sizeof(to_receive), 0)) < 1) {
      close(client_sock);
      return -1;
    }
    to_receive -= bytes;
    buffer_pos += bytes;
  }
  memcpy(&join_response, buffer, sizeof(join_response));

#ifdef DEBUG
  printf("status : 0x%4.4X\n", ntohs(join_response.status));
#endif

  if(ntohs(join_response.status) != JOIN_ACC) {
    close(client_sock);
    return -2;
  }

  p2p->local_addr = l_addr;
  p2p->client_sock = client_sock;
  return client_sock;
}

void p2p_close(p2p_struct_t* p2p) {
  close(p2p->client_sock);
  close(p2p->server_sock);
}

int p2p_listen(p2p_struct_t* p2p, char* host, char* port) {
	int call_result;
	int server_sock;
	struct sockaddr_in server_addr;
	int port_as_int;

	port_as_int = atoi(port);
	if(port_as_int <= 0) {
		port_as_int = PORT_DEFAULT;
	}

	/* Create the socket, see p2p_join for an explanation of the parameters. */
	if((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		return server_sock;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_as_int);
	server_addr.sin_addr.s_addr = inet_addr(host);
	if(server_addr.sin_addr.s_addr == INADDR_NONE) {
		server_addr.sin_addr.s_addr = INADDR_ANY;
	}

	if((call_result = bind(server_sock, (struct sockaddr*) &server_addr,
					sizeof(server_addr))) != 0) {
		close(server_sock);
		return call_result;
	}

	if((call_result = listen(server_sock, MAX_WAITING_CLIENTS)) < 0) {
		close(server_sock);
		return call_result;
	}

	p2p->server_sock = server_sock;
	p2p->server_addr = server_addr;
	return server_sock;
}

int p2p_accept(p2p_struct_t* p2p) {
	int peer_socket;
	struct sockaddr_in peer_addr;

	/* accept a peer */
	if((peer_socket = accept(p2p->server_sock, (struct sockaddr*) &peer_addr,
												sizeof(peer_addr))) < 0) {
		close(peer_socket);
	}

	return peer_socket;
}
