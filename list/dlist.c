#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dlist.h"

typedef struct _dlist_node {
    struct _dlist_node *prev;
    struct _dlist_node *next;
    void *data;
} dlist_node_t;

typedef struct _privinfo
{
    dlist_node_t *first;
} privinfo_t;

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

static dlist_node_t *dlist_get_node(list_t *thiz, int index, int fail_return_last)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *node = priv->first;

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

int dlist_length(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *iter = priv->first;
    int length = 0;

    while (iter != NULL)
    {
        iter = iter->next;
        length++;
    }

    return length;
}

int dlist_insert(list_t *thiz, int index, void *data)
{
    dlist_node_t *node = NULL;
    dlist_node_t *cursor = NULL;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    node = dlist_node_create(data);
    if (node == NULL)
    {
        dbg("Node create failed\n");
        return -1;
    }

    if (priv->first == NULL)
    {
        priv->first = node;
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
        if (cursor == priv->first)
        {
            priv->first = node;
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

int dlist_remove(list_t *thiz, int index)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *cursor = dlist_get_node(thiz, index, 0);

    if (cursor != NULL)
    {
        if (cursor == priv->first)
        {
            priv->first = cursor->next;
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

void dlist_print(list_t *thiz, data_print_fn print_fn)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *iter = priv->first;

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

int dlist_append(list_t *thiz, void *data)
{
    return dlist_insert(thiz, -1, data);
}

int dlist_prepend(list_t *thiz, void *data)
{
    return dlist_insert(thiz, 0, data);
}

void dlist_reverse(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *iter = priv->first;
    dlist_node_t *next = NULL;

    while (iter != NULL)
    {
        next = iter->next;
        iter->next = iter->prev;
        iter->prev = next;
        priv->first = iter;
        iter = next;
    }

    return;
}

void dlist_destroy(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    dlist_node_t *iter = priv->first;
    dlist_node_t *next = NULL;

    while (iter != NULL)
    {
        next = iter->next;
        dlist_node_destroy(iter);
        iter = next;
    }

    priv->first = NULL;
    free(thiz);

    return;
}

list_t *dlist_create(void)
{
    list_t *thiz = (list_t *)malloc(sizeof(list_t) + sizeof(privinfo_t));
    if (thiz != NULL)
    {
        privinfo_t *priv = (privinfo_t *)thiz->priv;

        thiz->destroy = dlist_destroy;
        thiz->insert = dlist_insert;
        thiz->get_length = dlist_length;
        thiz->append = dlist_append;
        thiz->prepend = dlist_prepend;
        thiz->remove = dlist_remove;
        thiz->reverse = dlist_reverse;
        thiz->dump = dlist_print;

        priv->first = NULL;
    }

    return thiz;
}

