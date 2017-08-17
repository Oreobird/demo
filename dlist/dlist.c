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

void dlist_destroy(dlist_t *thiz)
{
    dlist_node_t *node = NULL;
    dlist_node_t *next = NULL;
    
    if (thiz != NULL)
    {
        node = thiz->first;
        while (next != NULL)
        {
            next = node->next;
            dlist_node_destroy(node);
            node = next;
        }
    }

    return;
}

dlist_node_t *find_node_by_index(dlist_t *thiz, int index)
{
    dlist_node_t *node = thiz->first;

    while (node != NULL && index > 0) 
    {
        node = node->next;
        index--;
    }
    return node;
}

int dlist_insert(dlist_t *thiz, int index, void *data)
{
    dlist_node_t *node = NULL;
    dlist_node_t *slot = NULL; 

    node = dlist_node_create(data);
    if (node == NULL)
    {
        dbg("Node create failed\n");
        return -1;
    }

    slot = find_node_by_index(thiz, index);
    if (slot == NULL)
    {
        dbg("Index out of range\n");
        return -1;
    }

    node->next = slot->next;
    node->next->prev = node;
    slot->next = node;
    node->prev = slot;

    return 0;
}

