#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "kernel_list.h"

typedef struct _uuid_node
{
    char uuid[33];
    int status;
    struct list_head list;
} uuid_node_t;

void uuid_list_init(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

uuid_node_t *uuid_node_create(char *uuid, int status)
{
    int len = 0;
    uuid_node_t *pNode = (uuid_node_t *)malloc(sizeof(uuid_node_t));
    if (pNode == NULL)
    {
        printf("malloc failed\n");
        return NULL;
    }

    len = sizeof(pNode->uuid);
    memcpy(pNode->uuid, uuid, len);
    pNode->uuid[len - 1] = '\0';
    pNode->status = status;

    return pNode;
}

int uuid_list_add_node(struct list_head *head, char *uuid, int status)
{
    uuid_node_t *pNode = uuid_node_create(uuid, status);
    if (pNode == NULL)
    {
        return -1;
    }

    list_add_tail(&pNode->list, head);

    return 0;
}

void uuid_list_del_node_by_uuid(struct list_head *head, char *uuid)
{
    uuid_node_t *pos = NULL;
    uuid_node_t *node = NULL;

    if (list_empty(head))
    {
        return;
    }

    list_for_each_entry_safe(pos, node, head, list)
    {
        if (strncmp(pos->uuid, uuid, sizeof(pos->uuid)) == 0)
        {
            list_del(&pos->list);

            if (pos)
            {
                free(pos);
            }
            break;
        }
    }
}

void uuid_list_replace_node_by_uuid(struct list_head *head, char *old_uuid, uuid_node_t *new_node)
{
    uuid_node_t *pos = NULL;

    if (list_empty(head))
    {
        return;
    }

    list_for_each_entry(pos, head, list)
    {
        if (strncmp(pos->uuid, old_uuid, sizeof(pos->uuid)) == 0)
        {
            list_replace(&pos->list, &new_node->list);
            if (pos)
            {
                free(pos);
                pos = NULL;
            }
            break;
        }
    }
}

void uuid_list_clean(struct list_head *head)
{
    uuid_node_t *pos = NULL;
    uuid_node_t *node = NULL;

    if (list_empty(head))
    {
        return;
    }

    list_for_each_entry_safe(pos, node, head, list)
    {
        list_del(&pos->list);

        if (pos)
        {
            free(pos);
        }
    }
}

void uuid_list_show(struct list_head *head)
{
    uuid_node_t *pos = NULL;

    list_for_each_entry(pos, head, list)
    {
        printf("uuid: %s, status: %d\n", pos->uuid, pos->status);
    }
}

int main(void)
{
    struct list_head uuid_list;
    int i = 0;
    uuid_node_t *pNode = uuid_node_create("abcd", 0);

    uuid_list_init(&uuid_list);

    for (i = 0; i < 5; i++)
    {
        char uuid[20] = {0};
        snprintf(uuid, sizeof(uuid), "%d%d%d", i, i, i);
        uuid_list_add_node(&uuid_list, uuid, i);
    }

    uuid_list_show(&uuid_list);

    uuid_list_del_node_by_uuid(&uuid_list, "333");

    uuid_list_show(&uuid_list);

    uuid_list_replace_node_by_uuid(&uuid_list, "222", pNode);

    uuid_list_show(&uuid_list);

    uuid_list_clean(&uuid_list);

    return 0;
}





