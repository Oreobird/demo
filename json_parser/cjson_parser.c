#include <stdlib.h>
#include "cJSON.h"
#include "cjson_parser.h"

typedef struct _priv_info
{
    cJSON *obj;
} priv_info_t;

static cJSON *cjson_get_obj(const char *json_str)
{
    cJSON *obj = cJSON_Parse(json_str);
    if (!obj)
    {
        return NULL;
    }

    return obj;
}

static int cjson_get_str_obj_val(cJSON *obj, const char **str)
{
    if (str == NULL)
    {
        return -1;
    }

    if (obj->type != cJSON_String)
    {
        return -1;
    }

    if (!obj->valuestring)
    {
        return -1;
    }

    *str = obj->valuestring;
    return 0;
}

static int cjson_get_int_obj_val(cJSON *obj, int *val)
{
    if (obj == NULL)
    {
        return -1;
    }

    if (obj->type != cJSON_Number)
    {
        return -1;
    }

    *val = obj->valueint;
    return 0;
}

static int cjson_get_uint_obj_val(cJSON *obj, unsigned int *val)
{
    if (obj == NULL)
    {
        return -1;
    }

    if (obj->type != cJSON_Number)
    {
        return -1;
    }

    *val = (unsigned int)obj->valuedouble;
    return 0;
}

static cJSON *cjson_get_obj_param(cJSON *obj, const char *name)
{
    cJSON *params_obj = cJSON_GetObjectItem(obj, "params");
    if (!params_obj)
    {
        return NULL;
    }

    if (params_obj->type != cJSON_Object)
    {
        return NULL;
    }

    cJSON *sub_obj = cJSON_GetObjectItem(params_obj, name);
    if (!sub_obj)
    {
        return NULL;
    }

    return sub_obj;
}

static cJSON *cjson_get_obj_result(cJSON *obj, const char *name)
{
    cJSON *result_obj = cJSON_GetObjectItem(obj, "result");
    if (!result_obj)
    {
        return NULL;
    }

    if (result_obj->type != cJSON_Object)
    {
        return NULL;
    }

    cJSON *sub_obj = cJSON_GetObjectItem(result_obj, name);
    if (!sub_obj)
    {
        return NULL;
    }

    return sub_obj;
}

static int cjson_get_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *field_obj = cJSON_GetObjectItem(priv->obj, name);

    if (!field_obj)
    {
        return -1;
    }

    ret = cjson_get_str_obj_val(field_obj, val);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_int_field(json_parser_t *thiz, const char *name, int *val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *field_obj = cJSON_GetObjectItem(priv->obj, name);
    if(!field_obj)
    {
        return -1;
    }

    ret = cjson_get_int_obj_val(field_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_uint_field(json_parser_t *thiz, const char *name, unsigned int *val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *field_obj = cJSON_GetObjectItem(priv->obj, name);
    if(!field_obj)
    {
        return -1;
    }

    ret = cjson_get_uint_obj_val(field_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_param_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *param_obj = NULL;
    param_obj = cjson_get_obj_param(priv->obj, name);
    if(param_obj == NULL)
    {
        return -1;
    }

    ret = cjson_get_str_obj_val(param_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_param_int_field(json_parser_t *thiz, const char *name, int *val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *param_obj = NULL;
    param_obj = cjson_get_obj_param(priv->obj, name);
    if(ret < 0 || param_obj == NULL)
    {
        return -1;
    }

    ret = cjson_get_int_obj_val(param_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_result_str_field(json_parser_t *thiz, const char *name, const char **val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *result_obj = NULL;
    result_obj = cjson_get_obj_result(priv->obj, name);
    if(result_obj == NULL)
    {
        return -1;
    }

    ret = cjson_get_str_obj_val(result_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static int cjson_get_result_int_field(json_parser_t *thiz, const char *name, int *val)
{
    int ret;
    priv_info_t *priv = (priv_info_t *)thiz->priv;

    cJSON *result_obj = NULL;
    result_obj = cjson_get_obj_result(priv->obj, name);
    if(result_obj == NULL)
    {
        return -1;
    }

    ret = cjson_get_int_obj_val(result_obj, val);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

static void cjson_parser_destroy(json_parser_t *thiz)
{
    if (thiz != NULL)
    {
        priv_info_t *priv = (priv_info_t *)thiz->priv;
        if (priv && priv->obj)
        {
            cJSON_Delete(priv->obj);
        }
    }
}

json_parser_t *cjson_parser_create(const char *json_str)
{
    json_parser_t *thiz = (json_parser_t *)malloc(sizeof(json_parser_t) + sizeof(priv_info_t));

    if (thiz != NULL)
    {
        priv_info_t *priv = (priv_info_t *)thiz->priv;

        thiz->get_str_field = cjson_get_str_field;
        thiz->get_int_field = cjson_get_int_field;
        thiz->get_uint_field = cjson_get_uint_field;
        thiz->get_param_str_field = cjson_get_param_str_field;
        thiz->get_param_int_field = cjson_get_param_int_field;
        thiz->get_result_str_field = cjson_get_result_str_field;
        thiz->get_result_int_field = cjson_get_result_int_field;

        thiz->destroy_obj = cjson_parser_destroy;

        priv->obj = cjson_get_obj(json_str);
    }
    return thiz;
}

