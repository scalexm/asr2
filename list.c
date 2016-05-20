#include "list.h"
#include <stdlib.h>
#include <assert.h>

struct iterator {
    void * data;
    struct iterator * prev;
    struct iterator * next;
};

struct list {
    size_t size;
    iterator_t * begin;
    iterator_t * end;
};

list_t * list_new() {
    list_t * l = malloc(sizeof(list_t));
    l->size = 0;
    l->end = malloc(sizeof(iterator_t));
    l->end->data = l->end->prev = l->end->next = NULL;
    l->begin = l->end;
    return l;
}

size_t list_size(list_t * l) {
    return l->size;
}

int empty(list_t * l) {
    return l->size == 0;
}

void list_push_back(list_t * l, void * data) {
    list_insert(l, l->end, data);
}

void list_push_front(list_t * l, void * data) {
    list_insert(l, l->begin, data);
}

void list_insert(list_t * l, iterator_t * pos, void * data) {
    iterator_t * new_it = malloc(sizeof(iterator_t));
    new_it->data = data;
    new_it->next = pos;
    new_it->prev = pos->prev;
    pos->prev = new_it;
    ++l->size;
    if (pos == l->begin)
        l->begin = new_it;
    else
        new_it->prev->next = new_it;
}

iterator_t * list_begin(list_t * l) {
    return l->begin;
}

iterator_t * list_end(list_t * l) {
    return l->end;
}

void list_erase(list_t * l, iterator_t * pos) {
    assert(pos != l->end);
    if (pos == l->begin) {
        l->begin = pos->next;
        l->begin->prev = NULL;
    } else {
        pos->prev->next = pos->next;
        pos->next->prev = pos->prev;
    }
    free(pos);
    --l->size;
}

void list_pop_back(list_t * l) {
    assert(l->size >= 1);
    list_erase(l, l->end->prev);
}

void list_pop_front(list_t * l) {
    assert(l->size >= 1);
    list_erase(l, l->begin);
}

void list_clear(list_t * l) {
    iterator_t * it = l->begin;
    while (it != l->end) {
        it = it->next;
        free(it->prev);
    }
    l->end->prev = NULL;
    l->begin = l->end;
    l->size = 0;
}

void list_free(list_t * l) {
    list_clear(l);
    free(l->begin);
    free(l);
}

iterator_t * iterator_next(iterator_t * it) {
    return it->next;
}

iterator_t * iterator_prev(iterator_t * it) {
    return it->prev;
}

void * iterator_deref(iterator_t * it) {
    return it->data;
}
