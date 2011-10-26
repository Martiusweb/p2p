#include "p2p.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "sfhash.h"

static void p2p_set_header(p2p_struct_t* p2p, struct P2P_h* header, uint8_t msg_type) {
  struct {
    uint16_t port;
    uint32_t ip;
    time_t time;
  } msg_id_seed;

  header->version = 1;
  header->ttl = 5;
  header->msg_type = msg_type;
  header->length = 0;

  header->org_port = ntohs(p2p->local_addr.sin_port);
  header->org_ip = p2p->local_addr.sin_addr.s_addr;

  msg_id_seed.port = header->org_port;
  msg_id_seed.ip   = header->org_ip;
  msg_id_seed.time = time(NULL);
  header->msg_id = SuperFastHash((const char*) &msg_id_seed, sizeof(msg_id_seed));
}

int p2p_join(p2p_struct_t* p2p, char* host, char* port) {
  struct sockaddr_in h_addr;
  struct sockaddr_in l_addr;
  int client_sock;
  int call_state;
  struct p2p_msg_join_req join;
  socklen_t l_add_len;

  memset(p2p, 0, sizeof(p2p));
  memset(&h_addr, 0, sizeof(h_addr));
  /* We connect using TCP/Ipv4 */
  h_addr.sin_family = AF_INET;
  /* Parse the IP and format it for the low-level struct */
  h_addr.sin_addr.s_addr = inet_addr(host);
  /* Parse the port string in integer (atoi) and convert to network byte order
   * (htons) */
  h_addr.sin_port = htons(atoi(port));

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

  /* TODO wait for reply ? */

  p2p->local_addr = l_addr;
  p2p->client_sock = client_sock;
  return client_sock;
}

void p2p_close(p2p_struct_t* p2p) {
  close(p2p->client_sock);
}
