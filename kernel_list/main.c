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

int uuid_list_add_node(struct list_head *head, char *uuid, int status)
{
	int len = 0;
	uuid_node_t *pNode = (uuid_node_t *)malloc(sizeof(uuid_node_t));
	if (pNode == NULL)
	{
		printf("malloc failed\n");
		return -1;
	}
	
	len = sizeof(pNode->uuid);
	memcpy(pNode->uuid, uuid, sizeof(pNode->uuid));
	pNode->uuid[len - 1] = '\0';
	pNode->status = status;
	
	list_add_tail(&pNode->list, head);
	
	return 0;
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
	
	uuid_list_init(&uuid_list);
	
	for (i = 0; i < 5; i++)
	{
		uuid_list_add_node(&uuid_list, "1234", 1);
	}

	uuid_list_show(&uuid_list);
	
	uuid_list_clean(&uuid_list);
	
	return 0;
}





