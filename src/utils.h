#ifndef PHP_CLIPS_UTILS
#define PHP_CLIPS_UTILS

#include "../config.h"
#include "bool.h"
#include <php.h>
#include "clips/clips.h"

#define CLIPS_PHP_CONTEXT_RETURN "php_return"

extern void* p_clips_env;
extern zval* pzv_context;

void convert_do2php(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
bool php_hash_exists(zval* pzv_array, const char* s_key);
void php_hash_get(zval* pzv_array, const char* key, zval* pzv_ret);
void php_array_get(zval* pzv_array, int i_index, zval* pzv_ret);
void process_fact(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
void process_multifields(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_name(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_address(void* pv_env, DATA_OBJECT data, zval* pzv_val);

/**
 * Call the PHP's function. Follow these rules to convert type to PHP:
 * FLOAT => float
 * INTEGER => int
 * SYMBOL => string
 * STRING => string
 * MULTIFIELD => array of string | PHP class if the field is a template
 * FACT_ADDRESS => int
 */
void call_php_function(zval** pzv_obj, const char* s_php_method, DATA_OBJECT_PTR pdo_return_val, void* pv_env, int i_begin, int i_argc);

#endif
