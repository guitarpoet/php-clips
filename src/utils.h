#ifndef PHP_CLIPS_UTILS
#define PHP_CLIPS_UTILS

#include <php.h>
#include "clips/clips.h"

#define CLIPS_PHP_CONTEXT_RETURN "php_return"

extern void* p_clips_env;
extern zval* pzv_context;

void convert_do2php(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
void php_hash_get(zval* pzv_array, const char* key, zval* pzv_ret);
void php_array_get(zval* pzv_array, int i_index, zval* pzv_ret);
void process_fact(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
void process_multifields(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_name(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_address(void* pv_env, DATA_OBJECT data, zval* pzv_val);

#endif
