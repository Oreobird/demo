#ifndef __IOT_JSON_PARSER__
#define __IOT_JSON_PARSEr__

#include "json_parser.h"

int iot_json_get_str_field(json_parser_t *thiz, const char *name, const char **val);
int iot_json_get_int_field(json_parser_t *thiz, const char *name, int *val);
int iot_json_get_uint_field(json_parser_t *thiz, const char *name, unsigned int *val);
int iot_json_get_param_str_field(json_parser_t *thiz, const char *name, const char **val);
int iot_json_get_param_int_field(json_parser_t *thiz, const char *name, int *val);
int iot_json_get_result_str_field(json_parser_t *thiz, const char *name, const char **val);
int iot_json_get_result_int_field(json_parser_t *thiz, const char *name, int *val);
void iot_json_parser_destroy(json_parser_t *thiz);

json_parser_t *iot_cjson_parser_create(const char *json_str);

#endif
