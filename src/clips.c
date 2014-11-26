#include "clips.h"
#include <stdio.h>
#include "src/clips/factmch.h"

static zend_function_entry clips_functions[] = {
    PHP_FE(clips_init, NULL)
    PHP_FE(clips_close, NULL)
    PHP_FE(clips_console, NULL)
    PHP_FE(clips_exec, NULL)
    PHP_FE(clips_load, NULL)
    {NULL, NULL, NULL}
};


PHP_FUNCTION(clips_console) {
	if(p_clips_env) {
		CommandLoop(p_clips_env);
	}
}

PHP_FUNCTION(clips_init) {
	p_clips_env = CreateEnvironment();
    RETURN_STRING("Hello World", 1);
}

PHP_FUNCTION(clips_load) {
	char* s_filename;
	int i_filename_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_filename, &i_filename_len) == FAILURE) {
		RETURN_FALSE;
	}

	php_printf("Loading rules file %s", s_filename);

	if(!EnvLoad(p_clips_env, s_filename)) {
		php_printf("Oh NO!");
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_FUNCTION(clips_exec) {
	if(p_clips_env) {
		RouterData(p_clips_env)->CommandBufferInputCount = 1;
		RouterData(p_clips_env)->AwaitingInput = TRUE;
		EnvAssertString(p_clips_env, "(hello world)\n");

		ExecuteIfCommandComplete(p_clips_env);
		long start = UNSPECIFIED, end = UNSPECIFIED, max = UNSPECIFIED;
		struct defmodule *p_module;
		p_module = ((struct defmodule *) EnvGetCurrentModule(p_clips_env));

		EnvFacts(p_clips_env,WDISPLAY,p_module,start,end,max);
	}
}

PHP_FUNCTION(clips_close) {
    RETURN_STRING("Goodbye World", 1);
}

zend_module_entry clips_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_CLIPS_EXTNAME,
    clips_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_CLIPS_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

