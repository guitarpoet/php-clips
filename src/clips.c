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


/*******************************************************************************
 *
 *  Function clips_init
 *
 *  This function will initialize the clips engine for php
 *
 *  @version 1.0
 *  @args
 *  	None
 *
 *******************************************************************************/

PHP_FUNCTION(clips_init) {
	p_clips_env = CreateEnvironment();
	RETURN_TRUE;
}

/*******************************************************************************
 *
 *  Function clips_close
 *
 *  This function will close the clips engine
 *
 *  @version 1.0
 *  @args
 *  	None
 *
 *******************************************************************************/

PHP_FUNCTION(clips_close) {
   	DestroyEnvironment(p_clips_env);
	p_clips_env = NULL;
	RETURN_TRUE;
}

/*******************************************************************************
 *
 *  Function clips_console
 *
 *  This function will start a command line console for clips
 *
 *  @version 1.0
 *  @args
 *  	None
 *
 *******************************************************************************/

PHP_FUNCTION(clips_console) {
	if(p_clips_env) {
		CommandLoop(p_clips_env);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/*******************************************************************************
 *
 *  Function clips_exec
 *
 *  This function will execute a clips rule file
 *
 *  @version 1.0
 *  @args
 *  	filename: The clips file to be executed
 *
 *******************************************************************************/

PHP_FUNCTION(clips_exec) {
	if(p_clips_env) {
		char* s_filename;
		int i_filename_len;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_filename, &i_filename_len) == FAILURE) {
			RETURN_FALSE;
		}
		EnvBatchStar(p_clips_env, s_filename);
		RETURN_TRUE;
	}
}

/*******************************************************************************
 *
 *  Function clips_load
 *
 *  This function will load a rules file to the clips
 *
 *  @version 1.0
 *  @args
 *  	filename: The clips file to load
 *
 *******************************************************************************/

PHP_FUNCTION(clips_load) {
	char* s_filename;
	int i_filename_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_filename, &i_filename_len) == FAILURE) {
		RETURN_FALSE;
	}

	if(!EnvLoad(p_clips_env, s_filename)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
