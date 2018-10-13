/*
State machine example.
Table-Driven-Approach
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {ACTIVE, WAITING, PAUSED, FINISHED, ERROR}STATE;
typedef STATE (*process_func)(STATE state);

typedef struct _STATE_TBL
{
	STATE state;	
	process_func fn_handler;
}STATE_TBL;

STATE do_active(STATE state)
{
	printf("fsm: state is active\n");
	return PAUSED; 	
}

STATE do_waiting(STATE state)
{
	printf("fsm: state is waiting\n");
	return ACTIVE;
}

STATE do_pause(STATE state)
{
	printf("fsm: state is paused\n");
	return ERROR;
}

STATE do_finish(STATE state)
{
	printf("fsm: state is finished\n");
	return FINISHED;
}

STATE do_error(STATE state)
{
	printf("fsm: state is error\n");
	return WAITING;
}

STATE_TBL state_tbl[] = 
{
	{ACTIVE, do_active},
	{WAITING, do_waiting},
	{PAUSED, do_pause},
	{FINISHED, do_finish},
	{ERROR, do_error}
};

int stat_num = sizeof(state_tbl) / sizeof(STATE_TBL);

int fsm(STATE current)
{	
	STATE next = ERROR;
	int indx = 0;

	for (; indx < stat_num; indx++)
	{
		if (current == state_tbl[indx].state)
		{
			if (state_tbl[indx].fn_handler)
			{
				next = state_tbl[indx].fn_handler(current);
			}
		}
	} 

	return next;	
}

int main()
{
	STATE state = WAITING;
	
	while (state != FINISHED)	
	{
		state = fsm(state);
	}

	return 0;
}
