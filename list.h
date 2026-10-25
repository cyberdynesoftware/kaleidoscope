#ifndef _LIST_H_
#define _LIST_H_

struct list {
  void *value;
  struct list *next;
};

int size(struct list *l);

struct list *new_list_element(void *v);
struct list *list_append_value(struct list *l, void *v);

struct list *new_map_element(const char *key, void *val);
struct list *map_insert(struct list *map, const char *key, void *val);
void *map_get(struct list *map, char *key);

#endif
