#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define STR_MAP(_code, _str)   \
    {   \
        .code = (_code),    \
        .str = (_str),    \
    }

struct str_map {
    int code;
    const char *str;
} str_map[] = {
    STR_MAP(0, "haha0"),
    STR_MAP(1, "haha1"),
};

static const char *code_to_str(int code)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(str_map); i++)
    {
        if (code == str_map[i].code)
        {
            return str_map[i].str;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(str_map); i++)
    {
        printf("%s\n", code_to_str(i)); 
    }
    return 0;
}
