#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "notifier.h"
#include "chain.h"

int publish(void)
{
	return event_chain_notify(EVENTCHAIN_INIT, NULL);
}