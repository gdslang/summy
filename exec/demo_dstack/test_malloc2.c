#include <stdlib.h>

struct node;

struct node {
	struct node *next;
};

int main(int argc, char **argv) {
  struct node *head = malloc(sizeof(struct node));
  head->next = NULL;
  for(int i = 0; i < argc; i++) {
  	struct node *next = malloc(sizeof(struct node));
    next->next = head;
    head = next;
  }
	return (int)head;
}
