#include <stdio.h>
#include <assert.h>
#include "dlist.h"

static void print_int(void *data)
{
    printf("%d ", (int)data);
}

int main(int argc, char *argv[])
{
    int i = 0;
    int n = 100;
    dlist_t *dlist = dlist_create();

    for (i = 0; i < n; i++)
    {
        assert(dlist_append(dlist, (void *)i) == 0);
    }

    for (i = 0; i < n; i++)
    {
        assert(dlist_prepend(dlist, (void *)i) == 0);
    }

    dlist_print(dlist, print_int);
    dlist_destroy(dlist);

    return 0;
}
