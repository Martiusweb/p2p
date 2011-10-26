#include "p2p.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int p2p_join(p2p_struct_t* p2p, char* host, char* port) {
  struct sockaddr_in h_addr;
  int client_sock;
  int call_state;

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

  p2p->client_sock = client_sock;
  return client_sock;
}

void p2p_close(p2p_struct_t* p2p) {
  close(p2p->client_sock);
}
