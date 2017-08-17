#ifndef __DLIST_H__
#define __DLIST_H__

typedef struct _dlist_node {
    struct _dlist_node *prev;
    struct _dlist_node *next;
    void *data;
} dlist_node_t;

typedef struct _dlist_t {
    dlist_node_t *first;
} dlist_t;

typedef void (*dlist_data_print_fn)(void *data);

dlist_t *dlist_create(void);
void dlist_destroy(dlist_t *thiz);
int dlist_insert(dlist_t *thiz, int index, void *data);
int dlist_delete(dlist_t *thiz, void *data);
int dlist_get_by_index(dlist_t *thiz, int index, void *data);
int dlist_set_by_index(dlist_t *thiz, int index, void *data);
int dlist_length(dlist_t *thiz);
void dlist_print(dlist_t *thiz, dlist_data_print_fn print_fn);

#endif
