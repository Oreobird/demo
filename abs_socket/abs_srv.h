#ifndef __ABS_SRV_H__
#define __ABS_SRV_H__

#define DEBUG

#ifdef DEBUG
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) printf(fmt, ##args)
#endif

#define MAX_CLIENT 10
#define BUFF_SIZE 1500

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct _abs_srv;
typedef struct _abs_srv abs_srv_t;

typedef int (*setup_fn)(abs_srv_t *thiz);
typedef int (*create_socket_fn)(abs_srv_t *thiz);
typedef int (*loop_fn)(abs_srv_t *thiz);
typedef void (*destroy_fn)(abs_srv_t *thiz);

struct _abs_srv {
    create_socket_fn create_socket;
    setup_fn setup;
    loop_fn loop;
    destroy_fn destroy;
    char priv[0];
};

static inline int srv_create_socket(abs_srv_t *thiz)
{
    if (thiz == NULL)
        return -1;
    return thiz->create_socket ? thiz->create_socket(thiz) : -1;
}

static inline int srv_setup(abs_srv_t *thiz)
{
    if (thiz == NULL)
        return -1;
    return thiz->setup ? thiz->setup(thiz) : -1;
}

static inline int srv_loop(abs_srv_t *thiz)
{
    if (thiz == NULL)
        return -1;
    return thiz->loop ? thiz->loop(thiz) : -1;
}

static inline void srv_destroy(abs_srv_t *thiz)
{
    if (thiz == NULL)
        return;
    if (thiz->destroy)
        thiz->destroy(thiz);
}
#endif
