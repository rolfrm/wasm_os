#include <iron/full.h>
#include <microio.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
#include "awsmvm.h"

// socket driver for wasm_os
// needs to use async sockets as the OS itself requires some amount of cooperation
// for threads.


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef enum {
  SOCKET_IP4 = AF_INET,
  SOCKET_IP6 = AF_INET6
}socket_address_family;

typedef enum {
  SOCKET_STREAM = SOCK_STREAM

}socket_type;

typedef struct{
  u8 ip[16];
  i32 port;
  socket_address_family family;
  socket_type type; 
}__attribute__((packed)) _addrinfo;


// char * name, char * service, addrinfo * hints, addrinfo * res
void _get_addr_info(stack * ctx){
  _addrinfo * _res =  awsm_pop_ptr(ctx);
  _addrinfo * _hints =  awsm_pop_ptr(ctx);
  UNUSED(_hints);
  char * _service = awsm_pop_ptr(ctx);
  char * _name = awsm_pop_ptr(ctx);
  

  struct addrinfo hints, *res;
  int status;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(_service, _name, &hints, &res)) != 0) {
    awsm_push_u32(ctx, 2); // push error
    return;
  }
  if(res != NULL){
    struct addrinfo * p = res;
    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    if (p->ai_family == AF_INET) { // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      memcpy(_res->ip, &ipv4->sin_addr, sizeof(ipv4->sin_addr));
      _res->family = SOCKET_IP4;
    } else { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      memcpy(_res->ip, &ipv6->sin6_addr, sizeof(ipv6->sin6_addr));
      _res->family = SOCKET_IP6;
    }
  }
  
  freeaddrinfo(res);
  awsm_push_u32(ctx, 0);
}

// i32 ai_family, i32 socktype, i32 protocol -> i32 fd
void _socket(stack * ctx){
  i32 protocol = awsm_pop_i32(ctx);
  i32 socktype = awsm_pop_i32(ctx);
  i32 family = awsm_pop_i32(ctx);
  i32 s = socket(family, socktype, protocol);
  awsm_push_i32(ctx, s);
}

// i32 sockfd, void * addr, i32 addrlen
void _bind(stack * ctx){
  UNUSED(ctx);
}
