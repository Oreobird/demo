#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dlist.h"
#include "slist.h"

static void print_int(void *data)
{
    printf("%d ", (int)data);
}

int main(int argc, char **argv)
{
    list_t *list = NULL;
    int i = 0, n = 100;
    int ret = -1;

#if defined(TEST_DOUBLE_LIST)
    list = dlist_create();
#elif defined(TEST_SINGLE_LIST)
    list = slist_create();
#endif
    if (list == NULL)
        return -1;

    for (i = 0; i < n; i++)
    {
        assert(list_append(list, (void *)i) == 0);
    }

    for (i = 0; i < n; i++)
    {
        assert(list_prepend(list, (void *)i) == 0);
    }

    list_dump(list, print_int);
    list_reverse(list);
    list_dump(list, print_int);
    list_destroy(list);

	return 0;
}

