#include "notifier.h"

int notifier_chain_register(struct notifier_block **nl,
		struct notifier_block *n)
{
    while ((*nl) != NULL)
    {
        if ((*nl) == n)
            return 0;
        if (n->priority > (*nl)->priority)
            break;
        nl = &((*nl)->next);
    }

    n->next = *nl;
    *nl = n;

    return 0;
}

int notifier_chain_unregister(struct notifier_block **nl,
		struct notifier_block *n)
{
    while ((*nl) != NULL)
    {
        if ((*nl) == n)
        {
            *nl = n->next;
            return 0;
        }

        nl = &((*nl)->next);
    }

    return -1;
}

/**
 * notifier_call_chain - Informs the registered notifiers about an event.
 *	@nl:		Pointer to head of the blocking notifier chain
 *	@val:		Value passed unmodified to notifier function
 *	@v:		Pointer passed unmodified to notifier function
 *	@nr_to_call:	Number of notifier functions to be called. Don't care
 *			value of this parameter is -1.
 *	@nr_calls:	Records the number of notifications sent. Don't care
 *			value of this field is NULL.
 *	@returns:	notifier_call_chain returns the value returned by the
 *			last notifier function called.
 */
int notifier_call_chain(struct notifier_block **nl,
					unsigned long val, void *v)
{
    int ret = NOTIFY_DONE;
    struct notifier_block *nb, *next_nb;

    nb = *nl;

    while (nb)
    {
        next_nb = nb->next;
        ret = nb->notifier_call(nb, val, v);

        if ((ret & NOTIFY_STOP_MASK) == NOTIFY_STOP_MASK)
            break;

        nb = next_nb;
    }

    return ret;
}
