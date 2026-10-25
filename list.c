#include "list.h"
#include <stdlib.h>
#include <string.h>

int size(struct list *l) {
  int result = 0;
  while (l->next != NULL) {
    result++;
    l = l->next;
  }
  return result;
}

struct list *last_element(struct list *l) {
  struct list *e = l;
  while (e->next != NULL) {
    e = e->next;
  }
  return e;
}

struct list *new_list_element(void *v) {
  struct list *result = malloc(sizeof(struct list));
  result->value = v;
  result->next = NULL;
  return result;
}

struct list *list_append_value(struct list *l, void *v) {
  last_element(l)->next = new_list_element(v);
  return l;
}

struct list *new_map_element(const char *key, void *val) {
  struct list *value = malloc(sizeof(struct list));
  value->value = val;
  value->next = NULL;
  
  struct list *result = malloc(sizeof(struct list));
  result->value = key;
  result->next = value;
  return result;
}

struct list *map_insert(struct list *map, const char *key, void *val) {
  last_element(map)->next = new_map_element(key, val);
  return map;
}

void *map_get(struct list *map, char *key) {
  struct list *e = map;
  while (e != NULL && strcmp(e->value, key) != 0) {
    e = e->next->next;
  }
  if (e == NULL) {
    return NULL;
  } else {
    return e->next->value;
  }
}
