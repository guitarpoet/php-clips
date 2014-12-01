#ifndef PHP_CLIPS_UTILS
#define PHP_CLIPS_UTILS

#include <php.h>
#include "clips/clips.h"

extern void* p_clips_env;
extern zval* pzv_context;

void convert_do2php(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
void process_data_object_instance(void* pv_env, const char* s_instance_name, zval* pzv_val);
void process_fact(void* p_clips_env, DATA_OBJECT data, zval* pzv_val);
void process_multifields(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_name(void* pv_env, DATA_OBJECT data, zval* pzv_val);
void process_instance_address(void* pv_env, DATA_OBJECT data, zval* pzv_val);

#endif
