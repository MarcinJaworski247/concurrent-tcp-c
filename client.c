#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define SERVER_PORT 1068
#define SERVER_IP_ADDRESS "127.0.0.1"
#define MAXLINE 4096

void configure_connection(int socket_descriptor)
{
  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));

  server_address.sin_family = AF_INET;

  if(inet_pton(AF_INET, SERVER_IP_ADDRESS, &server_address.sin_addr) < 0) {
    perror("inet_pton");
  }
  if((server_address.sin_port=htons(SERVER_PORT)) < 0) {
    exit(-1);
  }

  if(connect(socket_descriptor, (struct sockaddr *)&server_address,
    sizeof(server_address)) < 0) {
      perror("connect");
    }
}

void communicate(int socket_descriptor)
{
  char buffer[MAXLINE];
  for (;;) {
    bzero(buffer, sizeof(buffer));
    scanf("%s", buffer);

    if(strcmp(buffer, "exit\n") == 0) {
      break;
    }

    if(write(socket_descriptor, buffer, sizeof(buffer)) < 0) {
      perror("write");
    }

    bzero(buffer, sizeof(buffer));

    if(read(socket_descriptor, buffer, sizeof(buffer)) < 0) {
      perror("read");
    }

    printf("from_server: %s\n", buffer);
  }
}

int main(void)
{
  int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_descriptor < 0) {
    perror("socket");
  }

  configure_connection(socket_descriptor);

  communicate(socket_descriptor);

  if(close(socket_descriptor) < 0) {
    perror("close");
  }

  return 0;
}
