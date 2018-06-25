#include <stdlib.h>

struct node;

struct node {
  struct node *next;
};

int main(int argc, char **argv) {
  struct node head;
  struct node *last = head.next;
  for(int i = 0; i < argc; i++) {
    last = last->next;
  }
  return (int)last;
}
