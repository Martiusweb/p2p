#ifndef P2P_H
#define P2P_H

typedef struct {
  int client_sock;
  void* user_data;
} p2p_struct_t;

/**
 * Connects to the peer host:port.
 *
 * Returns the connected socket or and error code (< 0).
 */
int p2p_join(p2p_struct_t* p2p, char* host, char* port);

/**
 * Close any connections.
 */
void p2p_close(p2p_struct_t* p2p);

#endif
