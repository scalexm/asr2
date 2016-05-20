#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct list list_t;
typedef struct iterator iterator_t;

list_t * list_new();
size_t list_size(list_t *);
int empty(list_t *);
void list_push_back(list_t *, void *);
void list_push_front(list_t *, void *);
void list_insert(list_t *, iterator_t *, void *);
iterator_t * list_begin(list_t *);
iterator_t * list_end(list_t *);
void list_erase(list_t *, iterator_t *);
void list_pop_back(list_t *);
void list_pop_front(list_t *);
void list_clear(list_t *);
void list_free(list_t *);

iterator_t * iterator_next(iterator_t *);
iterator_t * iterator_prev(iterator_t *);
void * iterator_deref(iterator_t *);

#endif
