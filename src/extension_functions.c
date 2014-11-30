#include "extension_functions.h"

void convert_do2php(DATA_OBJECT data, zval* pzv_val) {
	struct multifield *pmf_fields;
	struct deftemplate* template;
	switch(GetType(data)) {
		case FLOAT:
			ZVAL_DOUBLE(pzv_val, DOToDouble(data));
			break;
		case INTEGER:
			ZVAL_LONG(pzv_val, DOToLong(data));
			break;
		case INSTANCE_NAME:
			// This is an object, let's try to make is a class, if can't, make it an array
			break;
		case INSTANCE_ADDRESS:
			// This is an object, let's try to make is a class, if can't, make it an array
			break;
		case FACT_ADDRESS:
			template = (struct deftemplate *) FactDeftemplate(data.value);
			const char* s_template_name = ValueToString(template->header.name);

			zend_class_entry* pzce_class = zend_fetch_class(s_template_name, strlen(s_template_name), ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);
			if(pzce_class) {
				// We do have the class, let's create a php instance of it
				if(object_init_ex(pzv_val, pzce_class) == SUCCESS) {
					// Make the constructor zval
					zval* pzv_constructor;
					MAKE_STD_ZVAL(pzv_constructor);
					ZVAL_STRING(pzv_constructor, "__construct", TRUE);

					// Prepace the return value
					zval* pzv_ret_val;
					MAKE_STD_ZVAL(pzv_ret_val);

					// Let's call the construct method first
					if(call_user_function(EG(function_table), &pzv_val, pzv_constructor, pzv_ret_val, 0, NULL TSRMLS_CC) == SUCCESS) {
					}

					// Destroy the temporary zval variables
					zval_ptr_dtor(pzv_constructor);
					zval_ptr_dtor(pzv_ret_val);
				}
			}
			// We don't have the php class, let's make this fact an array
			break;
		case STRING:
		case SYMBOL:
			ZVAL_STRING(pzv_val, DOToString(data), TRUE);
			break;
		case MULTIFIELD:
			// Let's convert this to array
			array_init(pzv_val);
		   	pmf_fields = (struct multifield *) DOToPointer(data);
			// Iterate all the values in the multifields, and put them all into the array
			for(long i = EnvGetDOBegin(pv_env, data); i <= EnvGetDOEnd(pv_env, data); i++) {
				// Initialize the php variable as array item
				zval* pzv_array_item;
				MAKE_STD_ZVAL(pzv_array_item);
				
				switch(GetMFType(data.value, i)) {
					case INSTANCE_NAME:
						// This is an object, let's try to make is a class, if can't, make it an array
						break;
					case INSTANCE_ADDRESS:
						// This is an object, let's try to make is a class, if can't, make it an array
						break;
					case FACT_ADDRESS:
						// This is a fact, let's try to make is a class, if can't, make it an array
						break;
					case FLOAT:
						ZVAL_DOUBLE(pzv_array_item, ValueToDouble(GetMFValue(data.value, i)));
						break;
					case INTEGER:
						ZVAL_LONG(pzv_array_item, ValueToLong(GetMFValue(data.value, i)));
						break;
					case STRING:
					case SYMBOL:
						ZVAL_STRING(pzv_array_item, ValueToString(GetMFValue(data.value, i)), TRUE);
						break;
				}
				// Add it to the array
				add_next_index_zval(pzv_val, pzv_array_item);
			}
			break;
	}
}

/**
 * Call the PHP's function. The first argument must be string. Follow these rules to convert type to PHP:
 * FLOAT => float
 * INTEGER => int
 * SYMBOL => string
 * STRING => string
 * MULTIFIELD => array of string | PHP class if the field is a template
 * FACT_ADDRESS => int
 */
void php_call(void* pv_env, DATA_OBJECT_PTR pdo_return_val) {

	// Test the argument count is larger than 1
	if(EnvArgCountCheck(pv_env, "php_call", AT_LEAST, 1) == -1) {
		EnvSetpType(pv_env, pdo_return_val, STRING);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, ""));
		return ;
	}


	// Test if the first argument is string(function name)
	DATA_OBJECT do_php_function;
	if(!EnvArgTypeCheck(pv_env, "php_call", 1, STRING, &do_php_function)) {
		EnvSetpType(pv_env, pdo_return_val, STRING);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, ""));
		return ;
	}

	// Read the function name
	const char* str_php_function = DOToString(do_php_function);

	// Copy the function name to php variable
	zval* pzv_function_name;
	MAKE_STD_ZVAL(pzv_function_name);
	ZVAL_STRING(pzv_function_name, str_php_function, 1);

	// Get the parameters count
	int c = EnvRtnArgCount(pv_env) - 1;
	zend_uint i_param_count = c;

	// Initialize the paramter array
	zval** ppzv_params = (zval**) emalloc(c * sizeof(zval*));
	zval *pzv_php_ret_val;

	// Initialize the return php value
	MAKE_STD_ZVAL(pzv_php_ret_val);

	// Setup theinput parameters
	for(int i = 0; i < c; i++) {
		// Initialize the php value
		zval* val;
		MAKE_STD_ZVAL(val);

		// Getting the Data Object
		DATA_OBJECT o;

		EnvRtnUnknown(pv_env, 2 + i, &o); // Skipping the first once since it is the function name
		convert_do2php(o, val);
		ppzv_params[i] = val;
	}

	// Setup the default return value
	EnvSetpType(pv_env, pdo_return_val, STRING);
	EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, ""));

	// Call the functions
	if (call_user_function( EG(function_table), NULL /* no object */, pzv_function_name, pzv_php_ret_val, i_param_count, ppzv_params TSRMLS_CC) == SUCCESS) {
		switch(Z_TYPE_P(pzv_php_ret_val)) {
			case IS_LONG:
				EnvSetpType(pv_env, pdo_return_val, INTEGER);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddLong(pv_env, Z_LVAL_P(pzv_php_ret_val)));
				break;
			case IS_DOUBLE:
				EnvSetpType(pv_env, pdo_return_val, FLOAT);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddDouble(pv_env, Z_DVAL_P(pzv_php_ret_val)));
				break;
			case IS_ARRAY:
				// TODO Make this a multifiled
				break;
			case IS_OBJECT:
				// TODO Make this a fact(with template or not)
				break;
			case IS_STRING:
				EnvSetpType(pv_env, pdo_return_val, STRING);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, Z_STRVAL_P(pzv_php_ret_val)));
				break;
		}
	}

	// Destroy all the php parameter variables
	for(int i = 0; i < c; i++) {
		zval_ptr_dtor(&ppzv_params[i]);
	}
	// Destroy the php return variable
	zval_ptr_dtor(&pzv_php_ret_val);
	// Destroy the php function name variable
	zval_ptr_dtor(&pzv_function_name);
}
