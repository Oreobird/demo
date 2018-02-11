#include <stdlib.h>
#include <stdio.h>

#define LITTLE_END  (1)
#define BIG_END (0)

#if 1
int get_endian(void)
{
    int a = 0x01;
    char b = *((char *)&a);
    return (b == 0x01) ? LITTLE_END : BIG_END;
}
#else
int get_endian(void)
{
    union 
    {
        int a;
        char b;
    } un;

    un.a = 0x01;
    return (un.b == 0x01) ? LITTLE_END : BIG_END;
}
#endif

int main(void)
{
    int endian = get_endian();
    printf("cpu endian: %s\n", endian == LITTLE_END ? "little endian" : "big endian");
    return 0;
}
