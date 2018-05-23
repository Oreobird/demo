#ifndef _CHAIN_H_
#define _CHAIN_H_

#define EVENTCHAIN_INIT 0x11

int event_chain_register(struct notifier_block *n);
int event_chain_unregister(struct notifier_block *n);
int event_chain_notify(unsigned long val, void *v);

#endif
