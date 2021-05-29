#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 1068
#define MAXLINE 4096

void sig_child(int signo)
{
  pid_t pid;
  int stat;
  while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    printf("Child %d stopped\n", pid);
}

void configure_connection(int socket_descriptor)
{
  struct sockaddr_in server_address;
  bzero(&server_address, sizeof(server_address));

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htons(INADDR_ANY);
  server_address.sin_port = htons(PORT);

  if(bind(socket_descriptor, (struct sockaddr *)&server_address,
    sizeof(server_address)) < 0)
    {
      perror("bind");
    }

  if(listen(socket_descriptor, 16) < 0)
  {
    perror("listen");
  }
  signal(SIGCHLD, sig_child);
}

const char *changeOrder(char *msg)
{
  int i;
  int n = strlen(msg);

  int j = n-1;
  char *response;
  response = malloc(n);

  for(i = 0; i < n; i++) {
    response[i] = msg[j];
    j--;
  }

  response[i] = '\0';

  return response;
}

void responseToClient(int conn_file_desc)
{
  char buffer[MAXLINE];
  for(;;) {
    bzero(buffer, MAXLINE);

    if(read(conn_file_desc, buffer, MAXLINE) < 0)  {
      perror("read");
    }
    else {
      printf("from_client: %s\n", buffer);
    }

    const char *response = changeOrder(buffer);

    if(write(conn_file_desc, response, strlen(response)) < 0) {
      perror("write");
    }

  }
}

void communicate(int socket_descriptor)
{
  pid_t pid;

  for (;;) {
    int conn_file_desc = accept(socket_descriptor,
      (struct sockaddr *)NULL, NULL);

    if(conn_file_desc < 0)
    {
      if (errno == EINTR)
        continue;
      else
        perror("accept");
    }

    pid = fork();
    if(pid == 0) {
      responseToClient(conn_file_desc);
      if(close(socket_descriptor) < 0)
        perror("close");
    }
    else if(pid < 0)
      perror("fork");

    if(close(conn_file_desc) < 0)
      perror("close");
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

  return 0;
}
