#include "list.h"
#include <stdlib.h>

struct list *new_list_element(void *v) {
  struct list *result = malloc(sizeof(struct list));
  result->value = v;
  result->next = NULL;
  return result;
}

struct list *list_append_value(struct list *l, void *v) {
  struct list *e = l;
  while (e->next != NULL) {
    e = e->next;
  }
  e->next = new_list_element(v);
  return l;
}
