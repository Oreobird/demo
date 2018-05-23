#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "notifier.h"
#include "chain.h"
#include "common.h"

static int test_action_call(struct notifier_block *nb,
			                unsigned long action, void *data)
{
    UNUSED_VAL(nb);
    UNUSED_VAL(data);

    switch (action)
    {
        case EVENTCHAIN_INIT:
            printf("I got test chain init event\n");
            break;
        default:
            break;
    }

    return NOTIFY_OK;
}

static struct notifier_block nb = {
    	.notifier_call = test_action_call,
};

int subscribe(void)
{
    return event_chain_register(&nb);
}
