#include "extension_functions.h"

void process_instance_name(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
}

void process_instance_address(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
	const char* s_instance_name = EnvGetInstanceName(pv_env, DOToPointer(data));
	printf("The instance name is %s", s_instance_name);
}

/**
 * Process the multifields, turn them into an php array
 */
void process_multifields(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
	// Iterate all the values in the multifields, and put them all into the array
	for(long i = EnvGetDOBegin(pv_env, data); i <= EnvGetDOEnd(pv_env, data); i++) {
		// Initialize the php variable as array item
		zval* pzv_array_item;
		MAKE_STD_ZVAL(pzv_array_item);
		
		switch(GetMFType(data.value, i)) {
			case INSTANCE_NAME:
				// This is an object, let's try to make is a class, if can't, make it an array
				process_instance_name(pv_env, data, pzv_array_item);
				break;
			case INSTANCE_ADDRESS:
				// This is an object, let's try to make is a class, if can't, make it an array
				process_instance_address(pv_env, data, pzv_array_item);
				break;
			case FACT_ADDRESS:
				process_fact(pv_env, data, pzv_array_item);
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
}

/**
 * Process the fact, try class first, if no class is exists use array instead
 */
void process_fact(void* p_clips_env, DATA_OBJECT data, zval* pzv_val) {
	struct deftemplate* template = (struct deftemplate *) FactDeftemplate(data.value);
	const char* s_template_name = ValueToString(template->header.name);
	
	// The slots for the fact
	struct templateSlot* pts_slots = template->slotList;

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
				// The constructor is done, let's setting the properties
				while(pts_slots) {
					DATA_OBJECT do_slot_val;
					FactSlotValue(p_clips_env, data.value, ValueToString(pts_slots->slotName), &do_slot_val);

					const char* s_property_name = ValueToString(pts_slots->slotName);
					zval* pzv_property;
					MAKE_STD_ZVAL(pzv_property);

					// Convert the data object to php variable
					convert_do2php(p_clips_env, do_slot_val, pzv_property);

					// Put the property to the object
					zend_update_property(pzce_class, pzv_val, s_property_name, strlen(s_property_name), pzv_property);

					zval_ptr_dtor(&pzv_property); // Destroy the variable when the setting is done.

					// Move to next
					pts_slots = pts_slots->next;
				}
			}

			// Destroy the temporary zval variables
			zval_ptr_dtor(&pzv_constructor);
			zval_ptr_dtor(&pzv_ret_val);
			return;
		}
	}
	// We don't have the php class, let's make this fact an array
	array_init(pzv_val);

	// Process multifields first
	if(!pts_slots) {
		struct fact * pf_fact = DOToPointer(data);

		struct multifield* pmf_fields = (struct multifield*) pf_fact->theProposition.theFields[0].value;
		DATA_OBJECT do_tmp;
		SetDOBegin(do_tmp, 1);
		SetDOEnd(do_tmp, pmf_fields->multifieldLength);
		do_tmp.value = pmf_fields;

		process_multifields(p_clips_env, do_tmp, pzv_val);

	}

	zval* pzv_template_name;
	MAKE_STD_ZVAL(pzv_template_name);
	ZVAL_STRING(pzv_template_name, s_template_name, TRUE);

	// Then put the template name to the object
	add_assoc_zval(pzv_val, "template", pzv_template_name);
	// At last, let's adding the template slots
	while(pts_slots) {
		DATA_OBJECT do_slot_val;
		FactSlotValue(p_clips_env, data.value, ValueToString(pts_slots->slotName), &do_slot_val);

		const char* s_property_name = ValueToString(pts_slots->slotName);
		zval* pzv_property;
		MAKE_STD_ZVAL(pzv_property);

		// Convert the data object to php variable
		convert_do2php(p_clips_env, do_slot_val, pzv_property);

		// Put the property to the object
		add_assoc_zval(pzv_val, s_property_name, pzv_property);

		// Move to next
		pts_slots = pts_slots->next;
	}
}

void convert_do2php(void* p_clips_env, DATA_OBJECT data, zval* pzv_val) {
	struct multifield *pmf_fields;
	switch(GetType(data)) {
		case FLOAT:
			ZVAL_DOUBLE(pzv_val, DOToDouble(data));
			break;
		case INTEGER:
			ZVAL_LONG(pzv_val, DOToLong(data));
			break;
		case INSTANCE_NAME:
			// This is an object, let's try to make is a class, if can't, make it an array
			process_instance_name(p_clips_env, data, pzv_val);
			break;
		case INSTANCE_ADDRESS:
			// This is an object, let's try to make is a class, if can't, make it an array
			process_instance_address(p_clips_env, data, pzv_val);
			break;
		case FACT_ADDRESS:
			process_fact(p_clips_env, data, pzv_val);
			break;
		case STRING:
		case SYMBOL:
			if(strcmp(DOToString(data), "nil"))
				ZVAL_STRING(pzv_val, DOToString(data), TRUE);
			else
				ZVAL_NULL(pzv_val);
			break;
		case MULTIFIELD:
			// Let's convert this to array
			array_init(pzv_val);
			process_multifields(p_clips_env, data, pzv_val);
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
		convert_do2php(pv_env, o, val);
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
