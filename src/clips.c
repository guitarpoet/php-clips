#include "clips.h"

#ifdef COMPILE_DL_CLIPS
ZEND_GET_MODULE(clips)
#endif

static zend_function_entry clips_functions[] = {
    PHP_FE(clips_init, NULL)
    PHP_FE(clips_close, NULL)
    PHP_FE(clips_console, NULL)
    PHP_FE(clips_exec, NULL)
    PHP_FE(clips_load, NULL)
    PHP_FE(clips_is_command_complete, NULL)
	PHP_FE(clips_query_facts, NULL)
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
 *  	context: The context for the php clips execution
 *
 *******************************************************************************/

void* p_clips_env = NULL; // The global clips environment
zval* pzv_context = NULL; // The global php clips context

PHP_FUNCTION(clips_init) {

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &pzv_context) == SUCCESS) {
		p_clips_env = CreateEnvironment();
		RETURN_TRUE;
	}
	zend_error(E_ERROR, "No context setup for php clips!!!");
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
 *  This function will execute a clips command
 *
 *  @version 1.0
 *  @args
 *  	str: The clips command
 *
 *******************************************************************************/

PHP_FUNCTION(clips_exec) {
	if(p_clips_env) {
		char* s_str;
		int i_str_len;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_str, &i_str_len) == FAILURE) {
			RETURN_FALSE;
		}
		if (CommandLineData(p_clips_env)->BeforeCommandExecutionFunction != NULL) { 
			if (! (*CommandLineData(p_clips_env)->BeforeCommandExecutionFunction)(p_clips_env))
			{ RETURN_FALSE; }
		}

		FlushPPBuffer(p_clips_env);
		SetPPBufferStatus(p_clips_env,OFF);
		RouteCommand(p_clips_env,s_str,TRUE);
		FlushPPBuffer(p_clips_env);
		SetHaltExecution(p_clips_env,FALSE);
		SetEvaluationError(p_clips_env,FALSE);
		FlushCommandString(p_clips_env);

		CleanCurrentGarbageFrame(p_clips_env,NULL);
		CallPeriodicTasks(p_clips_env);
	}
	RETURN_FALSE;
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
	if(p_clips_env) {
		char* s_filename;
		int i_filename_len;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_filename, &i_filename_len) == FAILURE) {
			RETURN_FALSE;
		}
		EnvBatchStar(p_clips_env, s_filename);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/*******************************************************************************
 *
 *  Function clips_is_command_complete
 *
 *  This function will test if the command is the complete command string
 *
 *  @version 1.0
 *  @args
 *  	str: The command string to be tested
 *
 *******************************************************************************/

PHP_FUNCTION(clips_is_command_complete) {
	char* s_str;
	int i_str_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_str, &i_str_len) == FAILURE) {
		RETURN_FALSE;
	}
	if(CompleteCommand(s_str) == 0) {
		RETURN_FALSE;
	}
	else {
		RETURN_TRUE;
	}
}

/*******************************************************************************
 *
 *  Function clips_list_all_facts
 *
 *  This function will list all the facts in the clips environment
 *
 *  @version 1.0
 *  @args
 *  	template_name: The fact's template name, if not set, will return all the
 *  	facts
 *
 *******************************************************************************/

PHP_FUNCTION(clips_query_facts) {
	zval* pzv_facts;
	MAKE_STD_ZVAL(pzv_facts);
	array_init(pzv_facts);
	char* s_template_name = NULL;
	int i_template_name_len = 0;
	if(ZEND_NUM_ARGS()) {
		// We do have args
		zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_template_name, &i_template_name_len);
	}

	struct fact * pf_fact;
	// Let's iterate all the facts
	for (pf_fact = (struct fact *) EnvGetNextFact(p_clips_env, NULL);
			pf_fact != NULL;
			pf_fact = (struct fact *) EnvGetNextFact(p_clips_env, pf_fact)) {
		if(pf_fact) {
			struct deftemplate* pt_template = (struct deftemplate *) FactDeftemplate(pf_fact);
			if(strcmp("initial-fact", ValueToString(pt_template->header.name)) == 0 || // Skipping the initial-fact
				(s_template_name && strcmp(s_template_name, ValueToString(pt_template->header.name)) != 0)) {
				// We don't need to get this fact
				continue;
			}

			zval* pzv_array_item;
			MAKE_STD_ZVAL(pzv_array_item);
			DATA_OBJECT do_tmp;
			do_tmp.type = FACT_ADDRESS;
			do_tmp.value = pf_fact;
			process_fact(p_clips_env, do_tmp, pzv_array_item);

			add_next_index_zval(pzv_facts, pzv_array_item);
		}
	}
	RETURN_ZVAL(pzv_facts, TRUE, NULL);
}

/*******************************************************************************
 *
 *  Function clips_tempate_exists
 *
 *  This function will check if the template exists in the clips environment
 *
 *  @version 1.0
 *  @args
 *  	template_name: The template name to check
 *
 *******************************************************************************/

PHP_FUNCTION(clips_tempate_exists) {
	RETURN_FALSE
}

/*******************************************************************************
 *
 *  Function clips_instance_exists
 *
 *  This function will check if the instance name exists in the clips environment
 *
 *  @version 1.0
 *  @args
 *  	instance_name: The instance name to check
 *
 *******************************************************************************/

PHP_FUNCTION(clips_instance_exists) {
	RETURN_FALSE
}
