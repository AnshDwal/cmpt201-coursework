#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8001
#define BUF_SIZE 1024
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define NUM_MSG 5

static const char *messages[NUM_MSG] = {"Hello", "Apple", "Car", "Green",
                                        "Dog"};

int main() {
  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    handle_error("connect");
  }

  char buf[BUF_SIZE];
  for (int i = 0; i < NUM_MSG; i++) {
    sleep(1);
    // prepare message
    // this pads the desination with NULL
    strncpy(buf, messages[i], BUF_SIZE);

    if (write(sfd, buf, BUF_SIZE) == -1) {
      handle_error("write");
    } else {
      printf("Sent: %s\n", messages[i]);
    }
  }

  exit(EXIT_SUCCESS);
}
/*
Questions to answer at top of server.c:
(You should not need to change client.c)
Understanding the Client:
1. How is the client sending data to the server? What protocol?
    TCP socket over IPv4. It uses AF_INET which is IPv4 and SOCK_STREAM which is
a stream socket --> TCP. So it sends data using TCP protocol.
2. What data is the client sending to the server?
    It is sending an array of 5 strings --> Hello, Apple, Car, Green, Dog. For
each time it iterates through the array it send a buffer size, the beginning of
the buffer which is one of the strings and any padding used because of strncpy.
Understanding the Server:
1. Explain the argument that the `run_acceptor` thread is passed as an argument.
    Receives a pointer to a struct acceptor_args to access shared data safely.
The struct contains a run flag which tells the thread when to stop, a pointer to
a list (list_handle), and a pointer to list's mutex (list_lock).
2. How are received messages stored?
    Each message received by client tgread is copied into list_node and added to
a linked list. The list has a dummy head node, a count of how many messages in
total and a last pointer.
3. What does `main()` do with the received messages?
    It starts the acceptor thread & waits till expected number of messages have
been received. Once number of messages have been received, it stops the accepts
& calls collect_all() to print and free every messaged store in the. Then it
double checks if all emssages were collected before exiting.
4. How are threads used in this sample code?
    Once acceptor thread accepts clients conenctions and theres a different
client thread for each connected client. Each client thread reads messages from
its socket and adds them to the shared list. The main thread controls final
collection of messages, the start and the end.

5. Explain the use of non-blocking sockets in this lab.
   How are sockets made non-blocking?
    Sockets are made non-blocking by calling fcntl() to set O_NONBLOCK flag
   What sockets are made non-blocking?
    The sockets that are made non-blocking are the server listening socket (sfd)
and client connection socket (cfd) Why are these sockets made non-blocking? What
purpose does it serve? Sockets are made non-blocking to prevent server's threads
from getting stuck waiting for Input or Output. The acceptor thread will keep
looping even if no clients try to connect and each client thread would keep
looping even if client has not sent new data yet. The purpose it servers is
instead of blocking it lets accept() and read() return -1 which allows thread to
    continue running its loop. And, it makes it possible to check stop-flags and
shut down proeprly.
*/
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUF_SIZE 1024
#define PORT 8001
#define LISTEN_BACKLOG 32
#define MAX_CLIENTS 4
#define NUM_MSG_PER_CLIENT 5

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

struct list_node {
  struct list_node *next;
  void *data;
};

struct list_handle {
  struct list_node *last;
  volatile uint32_t count;
};

struct client_args {
  atomic_bool run;

  int cfd;
  struct list_handle *list_handle;
  pthread_mutex_t *list_lock;
};

struct acceptor_args {
  atomic_bool run;

  struct list_handle *list_handle;
  pthread_mutex_t *list_lock;
};

int init_server_socket() {
  struct sockaddr_in addr;

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  return sfd;
}

// Set a file descriptor to non-blocking mode
void set_non_blocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL");
    exit(EXIT_FAILURE);
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL");
    exit(EXIT_FAILURE);
  }
}

void add_to_list(struct list_handle *list_handle, struct list_node *new_node) {
  struct list_node *last_node = list_handle->last;
  last_node->next = new_node;
  list_handle->last = last_node->next;
  list_handle->count++;
}

int collect_all(struct list_node head) {
  struct list_node *node = head.next; // get first node after head
  uint32_t total = 0;

  while (node != NULL) {
    printf("Collected: %s\n", (char *)node->data);
    total++;

    // Free node and advance to next item
    struct list_node *next = node->next;
    free(node->data);
    free(node);
    node = next;
  }

  return total;
}

static void *run_client(void *args) {
  struct client_args *cargs = (struct client_args *)args;
  int cfd = cargs->cfd;
  set_non_blocking(cfd);

  char msg_buf[BUF_SIZE];

  while (cargs->run) {
    ssize_t bytes_read = read(cfd, &msg_buf, BUF_SIZE);
    if (bytes_read == -1) {
      if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
        perror("Problem reading from socket!\n");
        break;
      }
    } else if (bytes_read > 0) {
      // Create node with data
      struct list_node *new_node = malloc(sizeof(struct list_node));
      new_node->next = NULL;
      new_node->data = malloc(BUF_SIZE);
      memcpy(new_node->data, msg_buf, BUF_SIZE);

      struct list_handle *list_handle = cargs->list_handle;
      // TODO: Safely use add_to_list to add new_node to the list
      pthread_mutex_lock(cargs->list_lock);
      add_to_list(list_handle, new_node);
      pthread_mutex_unlock(cargs->list_lock);
    }
  }

  if (close(cfd) == -1) {
    perror("client thread close");
  }
  return NULL;
}

static void *run_acceptor(void *args) {
  int sfd = init_server_socket();
  set_non_blocking(sfd);

  struct acceptor_args *aargs = (struct acceptor_args *)args;
  pthread_t threads[MAX_CLIENTS];
  struct client_args client_args[MAX_CLIENTS];

  printf("Accepting clients...\n");

  uint16_t num_clients = 0;
  while (aargs->run) {
    if (num_clients < MAX_CLIENTS) {
      int cfd = accept(sfd, NULL, NULL);
      if (cfd == -1) {
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
          handle_error("accept");
        }
      } else {
        printf("Client connected!\n");

        client_args[num_clients].cfd = cfd;
        client_args[num_clients].run = true;
        client_args[num_clients].list_handle = aargs->list_handle;
        client_args[num_clients].list_lock = aargs->list_lock;
        num_clients++;

        // TODO: Create a new thread to handle the client
        if (pthread_create(&threads[num_clients - 1], NULL, run_client,
                           &client_args[num_clients - 1]) != 0) {
          perror("creating thread error");
          close(cfd);
        } else {
          num_clients++;
        }
      }
    }
  }
  printf("Not accepting any more clients!\n");

  // Shutdown and cleanup
  for (int i = 0; i < num_clients; i++) {
    // TODO: Set flag to stop the client thread
    // TODO: Wait for the client thread and close its socket
    client_args[i].run = false;
    if (pthread_join(threads[i], NULL) != 0) {
      perror("error doing pthread_join");
    }

    if (close(client_args[i].cfd) == -1) {
      perror("closing client socket");
    }
  }

  if (close(sfd) == -1) {
    perror("closing server socket");
  }
  return NULL;
}

int main() {
  pthread_mutex_t list_mutex;
  pthread_mutex_init(&list_mutex, NULL);

  // List to store received messages
  // - Do not free list head (not dynamically allocated)
  struct list_node head = {NULL, NULL};
  struct list_node *last = &head;
  struct list_handle list_handle = {
      .last = &head,
      .count = 0,
  };

  pthread_t acceptor_thread;
  struct acceptor_args aargs = {
      .run = true,
      .list_handle = &list_handle,
      .list_lock = &list_mutex,
  };
  pthread_create(&acceptor_thread, NULL, run_acceptor, &aargs);

  // TODO: Wait until enough messages are received
  while (1) {
    pthread_mutex_lock(&list_mutex);
    uint32_t c = list_handle.count;
    pthread_mutex_unlock(&list_mutex);

    if (c >= MAX_CLIENTS * NUM_MSG_PER_CLIENT) {
      break;
    }
  }
  aargs.run = false;
  pthread_join(acceptor_thread, NULL);

  if (list_handle.count != MAX_CLIENTS * NUM_MSG_PER_CLIENT) {
    printf("Not enough messages were received!\n");
    return 1;
  }

  int collected = collect_all(head);
  printf("Collected: %d\n", collected);
  if (collected != list_handle.count) {
    printf("Not all messages were collected!\n");
    return 1;
  } else {
    printf("All messages were collected!\n");
  }

  pthread_mutex_destroy(&list_mutex);

  return 0;
}
