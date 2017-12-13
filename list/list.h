#ifndef __LIST_H__
#define __LIST_H__

#define DEBUG
#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) do {} while(0)
#endif

struct _list;
typedef struct _list list_t;

typedef void (*data_print_fn)(void *data);

typedef void (*destroy_fn)(list_t *thiz);
typedef int (*insert_fn)(list_t *thiz, int index, void *data);
typedef int (*remove_fn)(list_t *thiz, int index);
typedef int (*get_length_fn)(list_t *thiz);
typedef int (*append_fn)(list_t *thiz, void *data);
typedef int (*prepend_fn)(list_t *thiz, void *data);
typedef void (*reverse_fn)(list_t *thiz);
typedef void (*dump_fn)(list_t *thiz, data_print_fn print_fn);

struct _list {
    destroy_fn destroy;
    insert_fn insert;
    remove_fn remove;
    get_length_fn get_length;
    append_fn append;
    prepend_fn prepend;
    reverse_fn reverse;
    dump_fn dump;

    char priv[0];
};

static inline void list_destroy(list_t *thiz)
{
    if (thiz == NULL)
        return;
    if (thiz->destroy)
        thiz->destroy(thiz);
}

static inline int list_insert(list_t *thiz, int index, void *data)
{
    if (thiz == NULL)
        return -1;
    return thiz->insert ? thiz->insert(thiz, index, data) : -1;
}

static inline int list_remove(list_t *thiz, int index)
{
    if (thiz == NULL)
        return -1;
    return thiz->remove ? thiz->remove(thiz, index) : -1;
}

static inline int list_get_length(list_t *thiz)
{
    if (thiz == NULL)
        return -1;
    return thiz->get_length ? thiz->get_length(thiz) : -1;
}

static inline int list_append(list_t *thiz, void *data)
{
    if (thiz == NULL)
        return -1;
    return thiz->append ? thiz->append(thiz, data) : -1;
}

static inline int list_prepend(list_t *thiz, void *data)
{
    if (thiz == NULL)
        return -1;
    return thiz->prepend ? thiz->prepend(thiz, data) : -1;
}

static inline void list_reverse(list_t *thiz)
{
    if (thiz == NULL)
        return;
    if (thiz->reverse)
        thiz->reverse(thiz);
}

static inline void list_dump(list_t *thiz, data_print_fn print_fn)
{
    if (thiz == NULL || print_fn == NULL)
        return;
    if (thiz->dump)
        thiz->dump(thiz, print_fn);
}
#endif
