#include <stdio.h>
#include <pthread.h>

struct s_net_status {
    int status;
    int (*set_net_status)(struct s_net_status *);
    int (*get_net_status)(void);
} g_net_status;


int main(int argc, char **argv)
{

    return 0;
}
