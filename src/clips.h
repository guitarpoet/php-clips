#ifndef PHP_CLIPS
#define PHP_CLIPS 1
#define PHP_CLIPS_VERSION "1.0"
#define PHP_CLIPS_EXTNAME "clips"
#define PHP_CLIPS_ENGINE_VERSION "6.30"

#include "../config.h"
#include "bool.h"
#include <stdio.h>
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>

#include "utils.h"
#include "clips/clips.h"

PHP_FUNCTION(clips_version);
PHP_FUNCTION(clips_init);
PHP_FUNCTION(clips_create_env);
PHP_FUNCTION(clips_switch_env);
PHP_FUNCTION(clips_meta);
PHP_FUNCTION(clips_close);
PHP_FUNCTION(clips_rules);
PHP_FUNCTION(clips_console);
PHP_FUNCTION(clips_load);
PHP_FUNCTION(clips_exec);
PHP_FUNCTION(clips_is_command_complete);
PHP_FUNCTION(clips_query_facts);
PHP_FUNCTION(clips_template_exists);
PHP_FUNCTION(clips_class_exists);
PHP_FUNCTION(clips_instance_exists);
PHP_MSHUTDOWN_FUNCTION(clips);

extern zend_module_entry clips_module_entry;
extern FILE * pf_stdout;
#define phpext_clips_ptr &clips_module_entry
#define UNSPECIFIED -1L

#endif
