#ifdef __x86_64__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#else

#include "kernel/stat.h"
#include "kernel/types.h"
#include "user/user.h"

typedef int pid_t;
typedef long long int int64_t;
typedef long long int ssize_t;
typedef unsigned long long int uint64_t;
typedef unsigned long long int size_t;
#endif

// Structures pipes into an understandable struct
typedef struct {
  int read;
  int write;
} pipes;

void pipe_read(pipes p, void *buf, size_t nbyte) { read(p.read, buf, nbyte); }

ssize_t pipe_write(pipes p, const void *buf, size_t nbyte) {
  return write(p.write, buf, nbyte);
}

void pipe_init(pipes *p) { pipe((void *)p); }

void pipe_close(pipes *p) {
  close(p->read);
  close(p->write);
}

void pipe_send(pipes output, const char *message) {
  uint64_t message_size = strlen(message) + 1;
  pipe_write(output, &message_size, sizeof(int64_t));
  pipe_write(output, message, message_size);
}

void pipe_recieve(pipes input) {
  uint64_t recieved_len;
  pipe_read(input, &recieved_len, sizeof(recieved_len));
  char *recieved_msg = malloc(recieved_len);
  pipe_read(input, recieved_msg, recieved_len);
  printf("process %d: got %s\n", getpid(), recieved_msg);
  free(recieved_msg);
}

// Parent process subroutine
void parent(pipes input, pipes output, char const *message) {
  pipe_send(output, message);
  pipe_recieve(input);
}

// Child process subroutine
void child(pipes input, pipes output, char const *message) {
  pipe_recieve(input);
  pipe_send(output, message);
}

int main(int argc, char **argv) {
  const char *ping = "ping";
  const char *pong = "pong";
  pipes pip_to_parent, pip_to_child;
  pipes *pip1_ptr = &pip_to_parent;
  pipe_init(pip1_ptr);

  pipes *pip2_ptr = &pip_to_child;
  pipe_init(pip2_ptr);

  pid_t pid = fork();
  if (pid == -1) {
    printf("fork failed");
    exit(1);
  } else if (pid == 0) {
    child(pip_to_child, pip_to_parent, pong);
  } else {
    parent(pip_to_parent, pip_to_child, ping);
  }

  pipe_close(&pip_to_parent);
  pipe_close(&pip_to_child);
  exit(0);
}
