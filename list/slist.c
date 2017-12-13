#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "slist.h"

typedef struct _slist_node {
    struct _slist_node *next;
    void *data;
} slist_node_t;

typedef struct _privinfo
{
    slist_node_t *first;
} privinfo_t;

static slist_node_t *slist_node_create(void *data)
{
    slist_node_t *node = NULL;

    node = (slist_node_t *)malloc(sizeof(slist_node_t));
    if (node != NULL)
    {
        node->next = NULL;
        node->data = data;
    }

    return node;
}

static void slist_node_destroy(slist_node_t *node)
{
    if (node != NULL)
    {
        node->next = NULL;
        free(node);
    }
}

static slist_node_t *slist_get_node(list_t *thiz, int index, int fail_return_last)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *node = priv->first;

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

int slist_length(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *iter = priv->first;
    int length = 0;

    while (iter != NULL)
    {
        iter = iter->next;
        length++;
    }

    return length;
}

int slist_insert(list_t *thiz, int index, void *data)
{
    slist_node_t *node = NULL;
    slist_node_t *cursor = NULL;
    privinfo_t *priv = (privinfo_t *)thiz->priv;

    node = slist_node_create(data);
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

    cursor = slist_get_node(thiz, index, 1);
    if (cursor == NULL)
    {
        dbg("Index out of range\n");
        return -1;
    }

    if (index < slist_length(thiz))
    {
        slist_node_t *temp = cursor->next;

        if (cursor == priv->first)
        {
            priv->first = node;
        }
        else
        {
            cursor->next = node;
        }
        node->next = temp;
    }
    else
    {
        cursor->next = node;
    }

    return 0;
}

int slist_remove(list_t *thiz, int index)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *cursor = slist_get_node(thiz, index - 1, 0);

    if (cursor != NULL)
    {
        slist_node_t *temp = cursor->next;

        if (cursor == priv->first)
        {
            priv->first = cursor->next;
        }

        if (cursor->next != NULL)
        {
            cursor->next = temp->next;
        }

        slist_node_destroy(temp);
    }

    return 0;
}

void slist_print(list_t *thiz, data_print_fn print_fn)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *iter = priv->first;

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

int slist_append(list_t *thiz, void *data)
{
    return slist_insert(thiz, -1, data);
}

int slist_prepend(list_t *thiz, void *data)
{
    return slist_insert(thiz, 0, data);
}

void slist_reverse(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *r, *s, *t = NULL;

    s = priv->first;
    if (s == NULL)
        return;

    while (s->next != NULL)
    {
        t = s->next;
        s->next = r;
        r = s;
        s = t;
    }

    priv->first = r;
    return;
}

void slist_destroy(list_t *thiz)
{
    privinfo_t *priv = (privinfo_t *)thiz->priv;
    slist_node_t *iter = priv->first;
    slist_node_t *next = NULL;

    while (iter != NULL)
    {
        next = iter->next;
        slist_node_destroy(iter);
        iter = next;
    }

    priv->first = NULL;
    free(thiz);

    return;
}

list_t *slist_create(void)
{
    list_t *thiz = (list_t *)malloc(sizeof(list_t) + sizeof(privinfo_t));
    if (thiz != NULL)
    {
        privinfo_t *priv = (privinfo_t *)thiz->priv;

        thiz->destroy = slist_destroy;
        thiz->insert = slist_insert;
        thiz->get_length = slist_length;
        thiz->append = slist_append;
        thiz->prepend = slist_prepend;
        thiz->remove = slist_remove;
        thiz->reverse = slist_reverse;
        thiz->dump = slist_print;

        priv->first = NULL;
    }

    return thiz;
}

