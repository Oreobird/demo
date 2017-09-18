#include <stdlib.h>
#include "cjson_parser.h"
#include "iot_json_parser.h"


json_parser_t *iot_cjson_parser_create(const char *json_str)
{
    return cjson_parser_create(json_str);
}

int iot_json_get_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_str_field ? thiz->get_str_field(thiz, name, val) : -1;
}

int iot_json_get_int_field(json_parser_t *thiz, const char *name, int *val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_int_field ? thiz->get_int_field(thiz, name, val) : -1;
}

int iot_json_get_uint_field(json_parser_t *thiz, const char *name, unsigned int *val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_uint_field ? thiz->get_uint_field(thiz, name, val) : -1;
}

int iot_json_get_param_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_param_str_field ? thiz->get_param_str_field(thiz, name, val) : -1;
}

int iot_json_get_param_int_field(json_parser_t *thiz, const char *name, int *val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_param_int_field ? thiz->get_param_int_field(thiz, name, val) : -1;
}

int iot_json_get_result_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_result_str_field ? thiz->get_result_str_field(thiz, name, val) : -1;
}

int iot_json_get_result_int_field(json_parser_t *thiz, const char *name, int *val)
{
    if (thiz == NULL || name == NULL || val == NULL)
    {
        return -1;
    }

    return thiz->get_result_int_field ? thiz->get_result_int_field(thiz, name, val) : -1;
}

void iot_json_parser_destroy(json_parser_t *thiz)
{
    if (thiz != NULL && thiz->destroy_obj)
    {
        thiz->destroy_obj(thiz);
    }
}
