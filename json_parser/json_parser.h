#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

struct _json_parser;
typedef struct _json_parser json_parser_t;

typedef void *(*get_obj_fn)(const char *json_str);
typedef int (*get_str_fn)(json_parser_t *thiz, const char *name, const char **val);
typedef int (*get_int_fn)(json_parser_t *thiz, const char *name, int *val);
typedef int (*get_unit_fn)(json_parser_t *thiz, const char *name, unsigned int *val);
typedef int (*get_param_str_fn)(json_parser_t *thiz, const char *name, const char **val);
typedef int (*get_param_int_fn)(json_parser_t *thiz, const char *name, int *val);
typedef int (*get_result_str_fn)(json_parser_t *thiz, const char *name, const char **val);
typedef int (*get_result_int_fn)(json_parser_t *thiz, const char *name, int *val);
typedef int (*get_attr_str_fn)(json_parser_t *thiz, const char *name, const char **val);
typedef int (*get_attr_int_fn)(json_parser_t *thiz, const char *name, int *val);

typedef void (*destroy_obj_fn)(json_parser_t *thiz);

struct _json_parser
{
    get_str_fn get_str_field;
    get_int_fn get_int_field;
    get_unit_fn get_uint_field;
    get_param_str_fn get_param_str_field;
    get_param_int_fn get_param_int_field;
    get_result_str_fn get_result_str_field;
    get_result_int_fn get_result_int_field;

    destroy_obj_fn destroy_obj;

    char priv[0];
};

#endif
