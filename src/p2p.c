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
static void p2p_set_header(p2p_struct_t* p2p, p2p_header_t* header, uint8_t msg_type) {
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
  p2p_msg_join_req_t join;
  p2p_msg_join_response_t join_response;
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

	/* Time to populate the p2p struct */
  p2p->local_addr = l_addr;
  p2p->client_sock = client_sock;

  /* receive join acknowledgement, I except this type of message, if the message
   * is not the right one, the behavior may be undefined.
   *
   * This shall be updated to use a more flexible and bulletproof method, but
   * here it works. It will be cleaned up by design with a non-blocking (io
   * events) architecture.
   */
	if((call_state = p2p_read_fixed_message(p2p, (p2p_fixed_msg_t*) &join_response)) < 1) {
		close(client_sock);
		return call_state;
	}

#ifdef DEBUG
  printf("status : 0x%4.4X\n", ntohs(join_response.status));
#endif

  if(ntohs(join_response.status) != JOIN_ACC) {
    close(client_sock);
    return -2;
  }

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
  socklen_t addr_ln;

	/* accept a peer */
  if((peer_socket = accept(p2p->server_sock, (struct sockaddr*) &peer_addr,
          &addr_ln)) < 0) {
		close(peer_socket);
	}

	return peer_socket;
}

int p2p_read_fixed_message(p2p_struct_t* p2p, p2p_fixed_msg_t* message) {
	char buffer[sizeof(p2p_fixed_msg_t)];
	int bytes_recvd;

	if((bytes_recvd = recv(p2p->client_sock, buffer, sizeof(p2p_fixed_msg_t), 0)) < 1) {
		return bytes_recvd;
	}
	memcpy(message, buffer, bytes_recvd);
	return bytes_recvd;
}

int p2p_query(p2p_struct_t* p2p, char* key, size_t length) {
	char* query = (char*) malloc(sizeof(p2p_header_t)+length);

	memset(&query, 0, sizeof(query));
  p2p_set_header(p2p, (p2p_header_t*) query, MSG_QUERY);
	((p2p_header_t*) query)->length = length;
	
	/* copy the payload after the header */
	memcpy(query+sizeof(p2p_header_t), key, length);

	/* Send query */
  if(send(p2p->client_sock, &query, sizeof(query), 0) != sizeof(query)) {
		free(query);
		return -1;
	}

	free(query);
	return 0;
}

int p2p_alloc_read_message(p2p_struct_t* p2p, int socket, char** message) {
  char* message_buffer;
  char* body_buffer;
  int bytes_read;
  int bytes_recvd;
  p2p_header_t message_header;

  /* read header, try to get the message length */
  if((bytes_recvd = recv(socket, (char*) &message_header, sizeof(p2p_header_t),
          0)) < 1) {
    /* Cannot read */
    *message = 0;
    return -1;
  }
  bytes_read = bytes_recvd;

  /* update the structure to get the proper endianness */
  message_header.org_port = ntohs(message_header.org_port);
  message_header.length   = ntohs(message_header.length);
  message_header.org_ip   = ntohl(message_header.org_ip);
  message_header.msg_id   = ntohl(message_header.msg_id);

  /* Get the message length and allocate memory */
  message_buffer = (char*) malloc(sizeof(p2p_header_t) + message_header.length);

  /* Read the full message */
  body_buffer = message_buffer + sizeof(p2p_header_t);

  if((bytes_recvd = recv(socket, body_buffer, message_header.length, 0)) < 1) {
    free(message_buffer);
    *message = 0;
    return -1;
  }
  bytes_read += bytes_recvd;

  /* copy the header into the returned memory trunk */
  memcpy(message_buffer, &message_header, sizeof(p2p_header_t));

  *message = message_buffer;
  return bytes_read;
}

int p2p_read_query_hit(p2p_struct_t* p2p, p2p_qhit_t* query_hit) {
  int bytes_read;
  char* msg_buffer;
  p2p_header_t* header;
  char* body;
  p2p_qhit_resource_t* entries;
  uint16_t entry_idx;

  if((bytes_read = p2p_alloc_read_message(p2p, p2p->client_sock, &msg_buffer)) < 1) {
    return -1;
  }

  header = (p2p_header_t*) msg_buffer;
  body = msg_buffer + sizeof(p2p_header_t);

  if(header->msg_type != MSG_QHIT) {
    free(msg_buffer);
    return -2;
  }

  /* Decode the message... */
  /* ... read the number of entries */
  query_hit->nb_entries = ntohs((uint16_t) *body);

  if(query_hit->nb_entries < 1) {
    query_hit->entries = 0;
  }
  else {
    body += sizeof(uint32_t);
    /* allocate enough memory to read the resources */
    entries = (p2p_qhit_resource_t*) malloc(query_hit->nb_entries *
        sizeof(p2p_qhit_resource_t));

    /* read the entries */
    for(entry_idx = 0; entry_idx < query_hit->nb_entries; ++entry_idx) {
      entries[entry_idx].id = ntohs((uint16_t) *body);
      body += sizeof(uint32_t);
      entries[entry_idx].value = ntohl((uint32_t) *body);
      body += sizeof(uint32_t);
    }

    query_hit->entries = entries;
  }

  free(msg_buffer);
  return bytes_read;
}
