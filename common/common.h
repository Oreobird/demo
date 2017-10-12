#ifndef __COMMON_H__
#define __COMMON_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifndef TRUE
#define TRUE    (1)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#define UNUSED_VAL(x)   ((void)(x))

#define BIT(x) (1U << (x))
#define BIT32(x) (((uint32_t) 1) << (x))
#define SETBIT(reg, bit)    ((reg) |= BIT(bit))
#define CLEARBIT(reg, bit)  ((reg) &= ~(BIT(bit)))
#define READBIT(reg, bit)   ((reg) & (BIT(bit)))

#define LOW_BYTE(n)     ((uint8_t)((n) & 0xFF))
#define HIGH_BYTE(n)    ((uint8_t)(LOW_BYTE((n) >> 8)))
#define HIGH_LOW_TO_INT(high, low)  ((((uint16_t)(high)) << 8) + ((uint16_t)((low) & 0xFF)))
#define BYTE_0(n)   ((uint8_t)((n) & 0xFF))
#define BYTE_1(n)   ((uint8_t)(BYTE_0((n) >> 8)))
#define BYTE_2(n)   ((uint8_t)(BYTE_0((n) >> 16)))
#define BYTE_3(n)   ((uint8_t)(BYTE_0((n) >> 24)))

#define SIZEOF(x)   (sizeof(x) / sizeof((a)[0]))

#endif
