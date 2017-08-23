#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dlist.h"

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) do {} while(0)
#endif

static dlist_node_t *dlist_node_create(void *data)
{
    dlist_node_t *node = NULL;

    node = (dlist_node_t *)malloc(sizeof(dlist_node_t));
    if (node != NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        node->data = data;
    }

    return node;
}

static void dlist_node_destroy(dlist_node_t *node)
{
    if (node != NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        free(node);
    }
}

dlist_t *dlist_create(void)
{
    dlist_t *thiz = malloc(sizeof(dlist_t));
    if (thiz != NULL)
    {
        thiz->first = NULL;
    }
    return thiz;
}

static dlist_node_t *dlist_get_node(dlist_t *thiz, int index, int fail_return_last)
{
    dlist_node_t *node = thiz->first;

    while (node != NULL && index > 0) 
    {
        node = node->next;
        index--;
    }

    if (!fail_return_last)
    {
        node = index > 0 ? NULL : node;
    }

    return node;
}

int dlist_length(dlist_t *thiz)
{
    int length = 0;
    dlist_node_t *iter = thiz->first;

    while (iter != NULL)
    {
        iter = iter->next;
        length++;
    }
    return length;
}

int dlist_insert(dlist_t *thiz, int index, void *data)
{
    dlist_node_t *node = NULL;
    dlist_node_t *cursor = NULL; 

    node = dlist_node_create(data);
    if (node == NULL)
    {
        dbg("Node create failed\n");
        return -1;
    }

    if (thiz->first == NULL)
    {
        thiz->first = node;
        return 0;
    }

    cursor = dlist_get_node(thiz, index, 1);
    if (cursor == NULL)
    {
        dbg("Index out of range\n");
        return -1;
    }

    if (index < dlist_length(thiz))
    {
        if (cursor == thiz->first)
        {
            thiz->first = node;
        }
        else
        {
            cursor ->prev->next = node;
            node->prev = cursor->prev;
        }
        node->next = cursor;
        cursor->prev = node;
    }
    else
    {
        cursor->next = node;
        node->prev = cursor;
    }

    return 0;
}

int dlist_delete(dlist_t *thiz, int index)
{
    dlist_node_t *cursor = dlist_get_node(thiz, index, 0);

    if (cursor != NULL)
    {
        if (cursor == thiz->first)
        {
            thiz->first = cursor->next;
        }

        if (cursor->next != NULL)
        {
            cursor->next->prev = cursor->prev;
        }

        if (cursor->prev != NULL)
        {
            cursor->prev->next = cursor->next;
        }

        dlist_node_destroy(cursor);
    }

    return 0;
}

void dlist_print(dlist_t *thiz, dlist_data_print_fn print_fn)
{
    dlist_node_t *iter = thiz->first;

    while (iter != NULL)
    {
        if (print_fn)
        {
            print_fn(iter->data);
        }
        iter = iter->next;
    }

    return;
}

int dlist_append(dlist_t *thiz, void *data)
{
    return dlist_insert(thiz, -1, data);
}

int dlist_prepend(dlist_t *thiz, void *data)
{
    return dlist_insert(thiz, 0, data);
}

void dlist_destroy(dlist_t *thiz)
{
    dlist_node_t *iter = thiz->first;
    dlist_node_t *next = NULL;
    
    while (iter != NULL)
    {
        next = iter->next;
        dlist_node_destroy(iter);
        iter = next;
    }

    thiz->first = NULL;
    free(thiz);

    return;
}
