#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "p2p.h"

int main(int argc, char* argv[]) {
  /* options :
   *  -h (host)
   *    Host to join
   *    usage : -hHOST
   *    It does not resolve DNS !
   *
   *  -p (port)
   *    Join the peer at given port
   *    usage : -pPORT
   **/
  static char* options = "l:j:";

  /* Parse the cli arguments */
  int opt;
  char* host_ip;
  char* host_port;
  p2p_struct_t p2p;

  if(argc < 2) {
    fprintf(stderr, "Not enough arguments.\n");
    return -1;
  }

  while((opt = getopt(argc, argv, options)) != -1) {
    switch(opt)
    {
      case 'h':
        host_ip = optarg;
        break;
      case 'p':
        host_port = optarg;
        break;
      case '?':
      default:
        /* getopt parse error, getopt displays a meaningful message by itself */
        fprintf(stderr, "Something went wrong, sorry.\n");
        return -1;
    }
  }

  /* Start the work */
  printf("Trying to join peer at %s...\n", optarg);

  if(p2p_join(&p2p, host_ip, host_port) < 0) {
    fprintf(stderr, "Connection to peer failed. Aborting.\n");
    return 0;
  }

  p2p_close(&p2p);
  return 0;
}
