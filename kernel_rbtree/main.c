#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "rbtree.h"

#define NODES       10

struct test_node 
{
	int key;
	struct rb_node rb;
    
	int val;
};

static struct rb_root root = RB_ROOT;
static struct test_node nodes[NODES];

static void insert(struct rb_root *root, struct test_node *node)
{
	struct rb_node **new = &root->rb_node, *parent = NULL;
	int key = node->key;

	while (*new) 
    {
		parent = *new;
		if (key < rb_entry(parent, struct test_node, rb)->key)
			new = &parent->rb_left;
		else
			new = &parent->rb_right;
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, root);
}

static inline void erase(struct rb_root *root, struct test_node *node)
{
	rb_erase(&node->rb, root);
}

static void traverse(struct rb_root *root)
{
    int i = 0;
    struct test_node *tn;
    struct rb_node *node = rb_first(root);

    while (node) 
    {
        tn = rb_entry(node, struct test_node, rb);
        printf("[%2d] tn->key: %d, tn->val: %d\n", i, tn->key, tn->val);
        i++;
        node = rb_next(node);
    }
}

static struct test_node *find(struct rb_root *root, int key)
{
    int found = 0;
    struct test_node *tn = NULL;
    struct rb_node *node = root->rb_node;

    while (node)
    {
        tn = rb_entry(node, struct test_node, rb);
        if (key < tn->key)
        {
            node = node->rb_left;
        }
        else if (key > tn->key)
        {
            node = node->rb_right;
        }
        else
        {
            found = 1;
            break;
        }
    }

    if (found)
    {
        return rb_entry(node, struct test_node, rb);
    }

    return NULL;
}

static void init_nodes(void)
{
	int i;
	for (i = 0; i < NODES; i++)
    {
		nodes[i].key = random() % 1024;
		nodes[i].val = random() % 1024;
	}
}

static void dump_nodes(void)
{
    int i = 0;
    for (i = 0; i < NODES; i++)
    {
        printf("nodes[%2d].key: %4d, nodes[%2d].val: %4d\n",
                i, nodes[i].key, i, nodes[i].val);
    }
}

int main(void)
{
	int i = 0;
    struct test_node *tn = NULL;
    
	init_nodes();

	for (i = 0; i < NODES; i++) 
    {
        insert(&root, &nodes[i]);
	}

    traverse(&root);

    erase(&root, &nodes[1]);
    traverse(&root);

    nodes[1].key = 123;
    insert(&root, &nodes[1]);
    traverse(&root);

    tn = find(&root, 123);
    if (tn)
    {
        printf("found: tn->key: %d, tn->val: %d\n", tn->key, tn->val);
        erase(&root, tn);
        traverse(&root);
    }
    else
    {
        printf("key: 123, not found\n");
    }

    tn = find(&root, 222);
    if (tn)
    {        
        printf("found: tn->key: %d, tn->val: %d\n", tn->key, tn->val);
    }
    else
    {
         printf("key: 222, not found\n");
    }

	return 0;
}
