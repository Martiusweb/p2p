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
  static char* options = "lh:p:";

  /* Parse the cli arguments */
  int opt;
  int listen = 0;
  char* arg_ip;
  char* arg_port;
  p2p_struct_t p2p;

  if(argc < 2) {
    fprintf(stderr, "Not enough arguments.\n");
    return -1;
  }

  while((opt = getopt(argc, argv, options)) != -1) {
    switch(opt)
    {
      case 'h':
        arg_ip = optarg;
        break;
      case 'p':
        arg_port = optarg;
        break;
      case 'l':
        listen = 1;
        break;
      case '?':
      default:
        /* getopt parse error, getopt displays a meaningful message by itself */
        fprintf(stderr, "Something went wrong, sorry.\n");
        return -1;
    }
  }

  p2p_init(&p2p);

  /* Of course, this "if" branch is not meant to last, we want to use libevent (or
   * epoll) in order to work with one process only. */
  if(listen) {
    /* Listen to incoming connections */
    printf("Trying to listen on %s port %s...\n", arg_ip, arg_port);

    if(p2p_listen(&p2p, arg_ip, arg_port) < 0) {
      fprintf(stderr, "Unable to listen on given interface/port. Aborting.\n");
      p2p_close(&p2p);
      return -1;
    }

    /* ready to accept a connection */
    if(p2p_accept(&p2p) < 0) {
      p2p_close(&p2p);
      return -1;
    }

    /* I'm able to read query hints here */
  }
  else {
    /* Act as a client */
    printf("Trying to join peer at %s...\n", arg_ip);

    if(p2p_join(&p2p, arg_ip, arg_port) < 0) {
      fprintf(stderr, "Connection to peer failed. Aborting.\n");
      p2p_close(&p2p);
      return -1;
    }
  }

  p2p_close(&p2p);
  printf("Ok, all done, Bye !\n");

  return 0;
}
