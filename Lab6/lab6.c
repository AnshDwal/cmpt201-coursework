#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// example_1.c
#define ASSERT(expr)                                                           \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                        \
      exit(1);                                                                 \
    }                                                                          \
  }

#define TEST(expr)                                                             \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Test failed: %s\n", #expr);                             \
      exit(1);                                                                 \
    } else {                                                                   \
      printf("Test passed: %s\n", #expr);                                      \
    }                                                                          \
  }

typedef struct node {
  uint64_t data;
  struct node *next;
} node_t;

node_t *head = NULL;

void insert_sorted(uint64_t data) {
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  ASSERT(new_node != NULL);
  new_node->data = data;
  new_node->next = NULL;

  if (head == NULL || data < head->data) {
    new_node->next = head;
    head = new_node;
    return;
  }

  node_t *curr = head;
  node_t *prev = NULL;
  bool inserted = false;

  while (curr != NULL && !inserted) {
    if (data < curr->data) {
      prev->next = new_node;
      new_node->next = curr;
      inserted = true;
    } else {
      prev = curr;
      curr = curr->next;
    }
  }
  if (!inserted) {
    prev->next = new_node;
  }
}

int index_of(uint64_t data) {
  node_t *curr = head;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }

    curr = curr->next;
    index++;
  }

  return -1;
}

int main() {
  insert_sorted(1);
  insert_sorted(2);
  insert_sorted(5);
  insert_sorted(3);

  TEST(index_of(3) == 2);

  insert_sorted(0);
  insert_sorted(4);

  TEST(index_of(4) == 4);

  return 0;
}
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s\n", #expr);                        \
      fprintf(stderr, "  at %s:%d\n", __FILE__, __LINE__);                     \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define TEST(expr)                                                             \
  {                                                                            \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Test failed: %s\n", #expr);                             \
      exit(1);                                                                 \
    }                                                                          \
  }

typedef struct node {
  uint64_t data;
  struct node *next;
} node_t;

typedef struct info {
  uint64_t sum;
} info_t;

node_t *head = NULL;
info_t info = {0};

void insert_sorted(uint64_t data) {
  node_t *new_node = malloc(sizeof(node_t));
  ASSERT(new_node != NULL);
  new_node->data = data;
  new_node->next = NULL;

  if (head == NULL) {
    head = new_node;
  } else if (data < head->data) {
    new_node->next = head;
    head = new_node;
  } else {
    node_t *curr = head;
    node_t *prev = NULL;

    while (curr != NULL) {
      if (data < curr->data) {
        break;
      }

      prev = curr;
      curr = curr->next;
    }

    prev->next = new_node;
    new_node->next = curr;
  }
  info.sum += data;
}

int index_of(uint64_t data) {
  node_t *curr = head;
  int index = 0;

  while (curr != NULL) {
    if (curr->data == data) {
      return index;
    }

    curr = curr->next;
    index++;
  }

  return -1;
}

static uint64_t list_sum(void) {
  uint64_t tot = 0;
  node_t *temp = head;

  while (temp != NULL) {
    tot += temp->data;
    temp = temp->next;
  }
  return tot;
}

int main() {
  insert_sorted(1);
  ASSERT(info.sum == list_sum());
  insert_sorted(3);
  ASSERT(info.sum == list_sum());

  insert_sorted(5);
  ASSERT(info.sum == list_sum());
  insert_sorted(2);
  ASSERT(info.sum == list_sum());

  TEST(info.sum == 1 + 3 + 5 + 2);
  TEST(index_of(2) == 1);

  ASSERT(info.sum == list_sum());
  return 0;
}
