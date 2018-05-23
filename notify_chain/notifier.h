#ifndef _NOTIFIER_H_
#define _NOTIFIER_H_

#include <stdlib.h>

struct notifier_block;

typedef int (*notifier_fn_t)(struct notifier_block *nb,
			unsigned long action, void *data);

struct notifier_block
{
	notifier_fn_t notifier_call;
	struct notifier_block *next;
	int priority;
};

struct notifier_head
{
	struct notifier_block *head;
};

#define INIT_NOTIFIER_HEAD(name) do {	\
		(name)->head = NULL;		\
	} while (0)

#define NOTIFIER_INIT(name)	{				\
		.head = NULL }

#define NOTIFIER_HEAD(name)					\
	struct notifier_head name =				\
		NOTIFIER_INIT(name)

int notifier_chain_register(struct notifier_block **nl,
		struct notifier_block *nb);

int notifier_chain_unregister(struct notifier_block **nl,
		struct notifier_block *nb);

int notifier_call_chain(struct notifier_block **nl,
		unsigned long val, void *v);


#define NOTIFY_DONE		0x0000		/* Don't care */
#define NOTIFY_OK		0x0001		/* Suits me */
#define NOTIFY_STOP_MASK	0x8000		/* Don't call further */
#define NOTIFY_BAD		(NOTIFY_STOP_MASK|0x0002)
						/* Bad/Veto action */
/*
 * Clean way to return from the notifier and stop further calls.
 */
#define NOTIFY_STOP		(NOTIFY_OK|NOTIFY_STOP_MASK)

/* Encapsulate (negative) errno value (in particular, NOTIFY_BAD <=> EPERM). */
static inline int notifier_from_errno(int err)
{
	if (err)
		return NOTIFY_STOP_MASK | (NOTIFY_OK - err);

	return NOTIFY_OK;
}

/* Restore (negative) errno value from notify return value. */
static inline int notifier_to_errno(int ret)
{
	ret &= ~NOTIFY_STOP_MASK;
	return ret > NOTIFY_OK ? NOTIFY_OK - ret : 0;
}

#endif
