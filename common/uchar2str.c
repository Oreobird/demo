#include <stdio.h>

static void uch2str(unsigned char *in_char, int len, char *out_str)
{
    int i = 0; 

    for (i = 0; i < len; i++)
    {
        sprintf(out_str + 2*i, "%02X", in_char[i]);
    }
}

int main(int argc, char **argv)
{
    int i = 0;
    unsigned char test[16] = {0x03, 0x04};
    char buf[32] = {0};

    uch2str(test, sizeof(test), buf);

    printf("buf: %s\n", buf);
    return 0;
}
