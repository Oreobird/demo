#ifndef __EVG_COMMON_H__
#define __EVG_COMMON_H__


//#define CAL_TIME 1

#define DEBUG 1

#ifdef DEBUG
#define fa_dbg(fmt, args...)  printf(fmt, ##args)
#else
#define fa_dbg(fmt, args...) do{}while(0)
#endif


#endif
