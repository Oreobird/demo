#ifndef _EVG_MSG_H_
#define _EVG_MSG_H_

typedef struct _msg
{
	int len;
	char payload[2048];
} evg_msg_t;

#endif
