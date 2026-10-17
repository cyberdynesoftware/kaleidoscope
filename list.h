struct list {
  void *value;
  struct list *next;
};

struct list *new_list_element(void *v);

struct list *list_append_value(struct list *l, void *v);
