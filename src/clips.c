#include "clips.h"
#include "environment_list.h"

#ifdef COMPILE_DL_CLIPS
ZEND_GET_MODULE(clips)
#endif

static PHP_MINFO_FUNCTION(clips){
	php_info_print_table_start();
	php_info_print_table_row(2, "Revision", PHP_CLIPS_VERSION);
	php_info_print_table_row(2, "ClipsRevision", PHP_CLIPS_ENGINE_VERSION);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}

static zend_function_entry clips_functions[] = {
    PHP_FE(clips_version, NULL)
    PHP_FE(clips_init, NULL)
    PHP_FE(clips_create_env, NULL)
    PHP_FE(clips_switch_env, NULL)
    PHP_FE(clips_meta, NULL)
    PHP_FE(clips_close, NULL)
    PHP_FE(clips_console, NULL)
    PHP_FE(clips_exec, NULL)
    PHP_FE(clips_load, NULL)
    PHP_FE(clips_is_command_complete, NULL)
	PHP_FE(clips_query_facts, NULL)
	PHP_FE(clips_template_exists, NULL)
	PHP_FE(clips_instance_exists, NULL)
	PHP_FE(clips_class_exists, NULL)
	PHP_FE(clips_rules, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry clips_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_CLIPS_EXTNAME,
    clips_functions,
    NULL, // No initialize functions
    PHP_MSHUTDOWN(clips),
    NULL,
    NULL,
    PHP_MINFO(clips),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_CLIPS_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

/****************************************************************************
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
		create_env("MAIN"); // Create the MAIN environment
		if(switch_env("MAIN")) {
			RETURN_TRUE;
		}
	}
	zend_error(E_ERROR, "No context setup for php clips!!!");
	RETURN_FALSE;
}

/*******************************************************************************
 *
 *  Function clips_version
 *
 *  This function will show the version of clips
 *
 *  @version 1.0
 *
 *******************************************************************************/

PHP_FUNCTION(clips_version) {
	RETURN_STRING(PHP_CLIPS_ENGINE_VERSION, true);
}

/*******************************************************************************
 *
 *  Function clips_create_env
 *
 *  This function will create a new clips env
 *
 *  @version 1.0
 *  @args
 *  	name: The name of the env
 *
 *******************************************************************************/

PHP_FUNCTION(clips_create_env) {
	if(p_clips_env) { // Only create the env when inited
		char* s_env_name;
		int i_str_len;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_env_name, &i_str_len) == FAILURE) {
			RETURN_FALSE;
		}
		create_env(s_env_name);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

/*******************************************************************************
 *
 *  Function clips_switch_env
 *
 *  This function will switch the clips env to that env
 *
 *  @version 1.0
 *  @args
 *  	name: The name of the env
 *
 *******************************************************************************/

PHP_FUNCTION(clips_switch_env) {
	if(p_clips_env) { // Only create the env when inited
		char* s_env_name;
		int i_str_len;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_env_name, &i_str_len) == FAILURE) {
			RETURN_FALSE;
		}
		if(switch_env(s_env_name)) {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}

/****************************************************************************
 *
 *  Function clips_meta
 *
 *  This function will return the metadata of the current env 
 *
 *  @version 1.0
 *  @args
 *  	array: The reference object
 *
 *******************************************************************************/

PHP_FUNCTION(clips_meta) {
	zval* pzv_meta = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &pzv_meta) == SUCCESS) {
		// For the current context
		EnvironmentListNode node = current_env_list_node();
		if(node) {
			add_assoc_string(pzv_meta, "current", node->s_name, TRUE);

			zval* pzv_arr = NULL;
			MAKE_STD_ZVAL(pzv_arr);
			array_init(pzv_arr);
			EnvironmentListNode node = env_list;
			while(node) {
				add_next_index_string(pzv_arr, node->s_name, TRUE);
				node = node->next;
			}
			add_assoc_zval(pzv_meta, "envs", pzv_arr);
		}
		RETURN_TRUE;
	}
	zend_error(E_ERROR, "No refer array setup for clips meta!!!");
	RETURN_FALSE;
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
	destroy_envlist();
	p_clips_env = NULL;
	RETURN_TRUE;
}

/*******************************************************************************
 *
 *  Module shutdown function
 *
 *  This function will close the clips engine when php exits
 *
 *  @version 1.0
 *  @args
 *  	None
 *
 *******************************************************************************/

PHP_MSHUTDOWN_FUNCTION(clips) {
	destroy_envlist();
	p_clips_env = NULL;
	return SUCCESS;
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
		zend_bool zb_debug = FALSE;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sb", &s_str, &i_str_len, &zb_debug) == FAILURE) {
			RETURN_FALSE;
		}
		if (CommandLineData(p_clips_env)->BeforeCommandExecutionFunction != NULL) { 
			if (! (*CommandLineData(p_clips_env)->BeforeCommandExecutionFunction)(p_clips_env))
			{ RETURN_FALSE; }
		}

		FlushPPBuffer(p_clips_env);
		SetPPBufferStatus(p_clips_env,OFF);
		if(zb_debug) {
			RouteCommand(p_clips_env,s_str, TRUE);
		}
		else {
			RouteCommand(p_clips_env,s_str, FALSE);
		}
		FlushPPBuffer(p_clips_env);
		EnvSetHaltExecution(p_clips_env,FALSE);
		EnvSetEvaluationError(p_clips_env,FALSE);
		FlushCommandString(p_clips_env);

		CleanCurrentGarbageFrame(p_clips_env,NULL);
		CallPeriodicTasks(p_clips_env);
		RETURN_TRUE;
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
	char* s_template_name = NULL;
	int i_template_name_len = 0;
	zval* pzv_facts;

	if(ZEND_NUM_ARGS() == 0) // If didn't get the args, just return false;
		RETURN_FALSE;

	if(ZEND_NUM_ARGS() == 1) { // If only one argument is there, it must be the array
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &pzv_facts) != SUCCESS) {
			RETURN_FALSE;
		}
	}

	if(ZEND_NUM_ARGS() == 2) {
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", &pzv_facts, &s_template_name, &i_template_name_len) != SUCCESS) {
			RETURN_FALSE;
		}
	}

	struct fact * pf_fact;
	// Let's iterate all the facts
	for (pf_fact = (struct fact *) EnvGetNextFact(p_clips_env, NULL);
			pf_fact != NULL;
			pf_fact = (struct fact *) EnvGetNextFact(p_clips_env, pf_fact)) {
		if(pf_fact) {
			struct deftemplate* pt_template = (struct deftemplate *) EnvFactDeftemplate(p_clips_env, pf_fact);
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
 *  Function clips_template_exists
 *
 *  This function will check if the template exists in the clips environment
 *
 *  @version 1.0
 *  @args
 *  	template_name: The template name to check
 *
 *******************************************************************************/

PHP_FUNCTION(clips_template_exists) {
	if(p_clips_env) {
		char* s_template_name = NULL;
		int i_template_name_len = 0;
		// Let's get the template name
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_template_name, &i_template_name_len) == SUCCESS) {
			struct deftemplate * pt_template;
			for (pt_template = (struct deftemplate *) EnvGetNextDeftemplate(p_clips_env, NULL);
					pt_template != NULL;
					pt_template = EnvGetNextDeftemplate(p_clips_env, pt_template)) {
				if(strcmp(s_template_name, ValueToString(pt_template->header.name)) == 0) {
					RETURN_TRUE;
				}
			}
		}
	}
	RETURN_FALSE;
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
	if(p_clips_env) {
		char* s_instance_name = NULL;
		int i_instance_name_len = 0;
		// Let's get the instance name
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_instance_name, &i_instance_name_len) == SUCCESS) {
			if(EnvFindInstance(p_clips_env, NULL, s_instance_name, TRUE)) {
				RETURN_TRUE;
			}
		}
	}
	RETURN_FALSE;
}


/*******************************************************************************
 *
 *  Function clips_class_exists
 *
 *  This function will check if the class is defined in the clips environment
 *
 *  @version 1.0
 *  @args
 *  	class_name: The class name
 *
 *******************************************************************************/

PHP_FUNCTION(clips_class_exists) {
	if(p_clips_env) {
		char* s_class_name = NULL;
		int i_class_name_len = 0;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &s_class_name, &i_class_name_len) == SUCCESS) {
			if(EnvFindDefclass(p_clips_env, s_class_name)) {
				RETURN_TRUE;
			}
		}
	}
	RETURN_FALSE;
}

/****************************************************************************
 *
 *  Function clips_rules
 *
 *  This function will list all the defined rules in the environment
 *
 *  @version 1.0
 *  @args
 *  	array: The reference object
 *
 *******************************************************************************/

void clips_count_all_rules(void* env, struct constructHeader* header,void * pv_zval) {
	zval* pzv_arr = (zval*) pv_zval;
	if(header->name) {
		const char* s_name = header->name->contents;
		if(s_name)
			add_next_index_string(pzv_arr, s_name, TRUE);
	}
}

PHP_FUNCTION(clips_rules) {
	if(p_clips_env) { // Only create the env when inited
		zval* pzv_arr = NULL;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &pzv_arr) == SUCCESS) {
			DoForAllConstructs(p_clips_env, clips_count_all_rules, DefruleData(p_clips_env)->DefruleModuleIndex, FALSE, pzv_arr);
			RETURN_TRUE;
		}

		zend_error(E_ERROR, "No refer array setup for clips rules!!!");
	}
	RETURN_FALSE;
}
