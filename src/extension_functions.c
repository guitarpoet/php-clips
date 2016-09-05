#include "extension_functions.h"
#include <string.h>

void process_instance_name(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
	php_hash_get(pzv_context, DOToString(data), pzv_val);
}

bool php_hash_exists(zval* pzv_array, const char* s_key) {
	HashTable* ht = Z_ARRVAL_P(pzv_array);
	HashPosition position;
	zval **data = NULL;

	bool found = FALSE;
	// Iterating all the key and values in the context
    while(zend_hash_has_more_elements(ht)) {
		 zend_string *key = NULL;
		 uint  klen;
		 zend_ulong index;

		 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_STRING) {
			 if(strcmp(key->val, s_key) == 0) {
				 // We have the key
				 found = TRUE;
			 }
		 } 
         zend_hash_move_forward(ht);
	}
	return found;
}

void php_array_get(zval* pzv_array, int i_index, zval* pzv_ret) {
	HashTable* ht = Z_ARRVAL_P(pzv_array);
	HashPosition position;
	zval **data = NULL;

	// Iterating all the key and values in the context
    while(zend_hash_has_more_elements(ht)) {
		 zend_string *key = NULL;
		 uint  klen;
		 zend_ulong index;

         *data = zend_hash_get_current_data(ht);
		 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_LONG) {
			 if(index == i_index) {
				 // We have the object
				 *pzv_ret = **data;
			 }
		 } 
         zend_hash_move_forward(ht);
	}
}

void php_hash_get(zval* pzv_array, const char* s_key, zval* pzv_ret) {
	if(php_hash_exists(pzv_array, s_key)) {
		// We have the key TODO Using zend_hash_quick_find to make this faster
		HashTable* ht = Z_ARRVAL_P(pzv_array);
		HashPosition position;
		zval **data = NULL;

		// Iterating all the key and values in the context
        while(zend_hash_has_more_elements(ht)) {
			 zend_string *key = NULL;
			 uint  klen;
			 zend_ulong index;

            *data = zend_hash_get_current_data(ht);
			 if (zend_hash_get_current_key_ex(ht, &key, &index, &position) == HASH_KEY_IS_STRING) {
				 if(strcmp(key->val, s_key) == 0) {
					 // We have the object
					 *pzv_ret = **data;
					 return;
				 }
			 } 
             zend_hash_move_forward(ht);
		}
	}
	else {
		zend_error(E_WARNING, "The key %s is not found in the php array\n", s_key);
	}
	ZVAL_NULL(pzv_ret);
}

void process_instance_address(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
	php_hash_get(pzv_context, EnvGetInstanceName(pv_env, DOToPointer(data)), pzv_val);
}

/**
 * Process the multifields, turn them into an php array
 */
void process_multifields(void* pv_env, DATA_OBJECT data, zval* pzv_val) {
	// Iterate all the values in the multifields, and put them all into the array
	int i;
	for(i = EnvGetDOBegin(pv_env, data); i <= EnvGetDOEnd(pv_env, data); i++) {
		// Initialize the php variable as array item
		zval zv_array_item;
		
		switch(GetMFType(data.value, i)) {
			case INSTANCE_NAME:
				process_instance_name(pv_env, data, &zv_array_item);
				break;
			case INSTANCE_ADDRESS:
				process_instance_address(pv_env, data, &zv_array_item);
				break;
			case FACT_ADDRESS:
				process_fact(pv_env, data, &zv_array_item);
				break;
			case FLOAT:
				ZVAL_DOUBLE(&zv_array_item, ValueToDouble(GetMFValue(data.value, i)));
				break;
			case INTEGER:
				ZVAL_LONG(&zv_array_item, ValueToLong(GetMFValue(data.value, i)));
				break;
			case STRING:
			case SYMBOL:
				ZVAL_STRING(&zv_array_item, ValueToString(GetMFValue(data.value, i)));
				break;
		}
		// Add it to the array
		add_next_index_zval(pzv_val, &zv_array_item);
	}
}

/**
 * Process the fact, try class first, if no class is exists use array instead
 */
void process_fact(void* p_clips_env, DATA_OBJECT data, zval* pzv_val) {
	struct deftemplate* template = (struct deftemplate *) EnvFactDeftemplate(p_clips_env, data.value);
	const char* s_template_name = strdup(ValueToString(template->header.name));
	
	// The slots for the fact
	struct templateSlot* pts_slots = template->slotList;

    zend_string* pzs_template_name = zend_string_init(s_template_name, strlen(s_template_name), 0);

	zend_class_entry* pzce_class = zend_fetch_class(pzs_template_name, ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);
    zend_string_release(pzs_template_name);
	if(pzce_class) {
		// We do have the class, let's create a php instance of it
		if(object_init_ex(pzv_val, pzce_class) == SUCCESS) {
			// Make the constructor zval
			zval zv_constructor;
			ZVAL_STRING(&zv_constructor, "__construct");

			// Prepace the return value
			zval zv_ret_val;

			// Let's call the construct method first
			if(call_user_function(EG(function_table), pzv_val, &zv_constructor, &zv_ret_val, 0, NULL TSRMLS_CC) == SUCCESS) {
				// TODO: What to do if the object didn't have the constructor?
			}
			// The constructor is done, let's setting the properties
			while(pts_slots) {
				DATA_OBJECT do_slot_val;
				FactSlotValue(p_clips_env, data.value, strdup(ValueToString(pts_slots->slotName)), &do_slot_val);

				const char* s_property_name = strdup(ValueToString(pts_slots->slotName));
				zval zv_property;

				// Convert the data object to php variable
				convert_do2php(p_clips_env, do_slot_val, &zv_property);

				// Put the property to the object
				zend_update_property(pzce_class, pzv_val, s_property_name, strlen(s_property_name), &zv_property TSRMLS_CC);

				// Move to next
				pts_slots = pts_slots->next;
			}

			return;
		}
		ZVAL_NULL(pzv_val);
		return;
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

	// Then put the template name to the object
	add_assoc_string(pzv_val, "__template__", (char*) s_template_name);
	// At last, let's adding the template slots
	while(pts_slots) {
		DATA_OBJECT do_slot_val;
		FactSlotValue(p_clips_env, data.value, strdup(ValueToString(pts_slots->slotName)), &do_slot_val);

		const char* s_property_name = strdup(ValueToString(pts_slots->slotName));

        zval z;
		zval* zv_property = &z;

		// Convert the data object to php variable
		convert_do2php(p_clips_env, do_slot_val, zv_property);

		// Put the property to the object
		add_assoc_zval(pzv_val, s_property_name, zv_property);

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
			// This is an object, we'll direct link it into php's object variable
			process_instance_name(p_clips_env, data, pzv_val);
			break;
		case INSTANCE_ADDRESS:
			// This is an object, we'll direct link it into php's object variable
			process_instance_address(p_clips_env, data, pzv_val);
			break;
		case FACT_ADDRESS:
			process_fact(p_clips_env, data, pzv_val);
			break;
		case STRING:
		case SYMBOL:
			if(strcmp(DOToString(data), "nil") == 0) {
				ZVAL_NULL(pzv_val);
				return;
			}

			if(strcmp(DOToString(data), "TRUE") == 0) {
				ZVAL_TRUE(pzv_val);
				return;
			}

			if(strcmp(DOToString(data), "FALSE") == 0) {
				ZVAL_FALSE(pzv_val);
				return;
			}

			ZVAL_STRING(pzv_val, DOToString(data));
			break;
		case MULTIFIELD:
			// Let's convert this to array
			array_init(pzv_val);
			process_multifields(p_clips_env, data, pzv_val);
			break;
	}
}

void convert_php_array2multifield(void* pv_env, zval* pzv_array, DATA_OBJECT_PTR pdo_val) {
	EnvSetpType(pv_env, pdo_val, MULTIFIELD);
	// Create the multifield first
	long size = zend_hash_num_elements(Z_ARRVAL_P(pzv_array));
	struct multifields* pmf_fields = EnvCreateMultifield(pv_env, size);
	SetpDOBegin(pdo_val, 1);
	SetpDOEnd(pdo_val, size);

	// Iterating all the key and values in the array
	HashTable* ht = Z_ARRVAL_P(pzv_array);
	HashPosition position;
	zval **data = NULL;

	int i = 1;

    zend_hash_internal_pointer_reset_ex(ht, &position);

    while(zend_hash_has_more_elements(ht)) {
        *data = zend_hash_get_current_data(ht);
		switch(Z_TYPE_P(*data)) {
		case IS_NULL:
		case IS_ARRAY:
		case IS_OBJECT:
			// We don't support for array or object in the multifields, treat them as nil
			EnvSetMFType(pv_env, pmf_fields, i, SYMBOL);
			EnvSetMFValue(pv_env, pmf_fields, i, EnvAddSymbol(pv_env, "nil"));
			break;
		case IS_LONG:
			EnvSetMFType(pv_env, pmf_fields, i, INTEGER);
			EnvSetMFValue(pv_env, pmf_fields, i, EnvAddLong(pv_env, Z_LVAL_P(*data)));
			break;
		case IS_DOUBLE:
			EnvSetMFType(pv_env, pmf_fields, i, FLOAT);
			EnvSetMFValue(pv_env, pmf_fields, i, EnvAddLong(pv_env, Z_DVAL_P(*data)));
			break;
		case IS_STRING:
			EnvSetMFType(pv_env, pmf_fields, i, STRING);
			EnvSetMFValue(pv_env, pmf_fields, i, EnvAddSymbol(pv_env, strdup(Z_STRVAL_P(*data))));
			break;
		}
		i++;
        zend_hash_move_forward(ht);
    }

	SetpValue(pdo_val, pmf_fields);
}

/**
 * Call the php's object's method. Same as php call, but only try to call the method within the php object
 * that locates in the clips context
 */
void php_method(void* pv_env, DATA_OBJECT_PTR pdo_return_val) {
	// Test the argument count is larger than 1
	if(EnvArgCountCheck(pv_env, "php_method", AT_LEAST, 2) == -1) {
		EnvSetpType(pv_env, pdo_return_val, SYMBOL);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
		return ;
	}

	// The method argument must be string
	DATA_OBJECT do_php_method;
	if(!EnvArgTypeCheck(pv_env, "php_method", 2, STRING, &do_php_method)) {
		EnvSetpType(pv_env, pdo_return_val, SYMBOL);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
		return ;
	}

	// Test if the first argument is instance name or the instance address (object name in the context)
	DATA_OBJECT do_php_object_name;
	const char* s_object_name;
	EnvRtnUnknown(pv_env, 1, &do_php_object_name);
	switch(GetType(do_php_object_name)) {
	case INSTANCE_NAME:
		s_object_name = DOToString(do_php_object_name);
		break;
	case INSTANCE_ADDRESS:
		s_object_name = EnvGetInstanceName(pv_env, DOToPointer(do_php_object_name));
		break;
	default:
		EnvSetpType(pv_env, pdo_return_val, SYMBOL);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
		return ;
	}

	if(php_hash_exists(pzv_context, s_object_name)) {
		zval zv_obj;
        zval* pzv_obj = &zv_obj;
		php_hash_get(pzv_context, s_object_name, &zv_obj);
		call_php_function(&pzv_obj, DOToString(do_php_method), pdo_return_val, pv_env, 3, EnvRtnArgCount(pv_env) - 2);
	}
	else {
		char s_message[256];
		sprintf(s_message, "named %s in the clips context.", s_object_name);
		CantFindItemErrorMessage(pv_env, "PHP_OBJECT", s_message);
	}
}

void call_php_function(zval** ppzv_obj, const char* s_php_method, DATA_OBJECT_PTR pdo_return_val, void* pv_env, int i_begin, int i_argc) {
	// Copy the function name to php variable
	zval zv_function_name;
	ZVAL_STRING(&zv_function_name, s_php_method);

	zend_ulong i_param_count = i_argc;

	// Initialize the paramter array
	zval** ppzv_params = (zval**) emalloc(i_argc * sizeof(zval*));
	zval zv_php_ret_val;


	int* i_types = (int*) emalloc(i_argc * sizeof(int));

    zval vs[i_argc];
	// Setup the input parameters
	int i = 0;
	for(i = 0; i < i_argc; i++) {
		// Initialize the php value
		zval* val = &vs[i];

		// Getting the Data Object
		DATA_OBJECT o;

		EnvRtnUnknown(pv_env, i_begin + i, &o); // Skipping the first once since it is the function name
		convert_do2php(pv_env, o, val);
		i_types[i] = GetType(o);
		ppzv_params[i] = val;
	}

    zval* pzv_obj = ppzv_obj? *ppzv_obj : NULL;
	// Call the functions
	if (call_user_function(EG(function_table), pzv_obj, &zv_function_name, &zv_php_ret_val, i_param_count, *ppzv_params TSRMLS_CC) == SUCCESS) {
		switch(Z_TYPE_P(&zv_php_ret_val)) {
			case IS_LONG:
				EnvSetpType(pv_env, pdo_return_val, INTEGER);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddLong(pv_env, Z_LVAL_P(&zv_php_ret_val)));
				break;
			case IS_TRUE:
				EnvSetpType(pv_env, pdo_return_val, SYMBOL);
                EnvSetpValue(pv_env, pdo_return_val, EnvTrueSymbol(pv_env));
				break;
			case IS_FALSE:
				EnvSetpType(pv_env, pdo_return_val, SYMBOL);
                EnvSetpValue(pv_env, pdo_return_val, EnvFalseSymbol(pv_env));
			case IS_DOUBLE:
				EnvSetpType(pv_env, pdo_return_val, FLOAT);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddDouble(pv_env, Z_DVAL_P(&zv_php_ret_val)));
				break;
			case IS_ARRAY:
				// If this is an array, let's make a multifield with it, and we only support for normal value(NULL, INTEGER, FLOAT, STRING) here
				convert_php_array2multifield(pv_env, &zv_php_ret_val, pdo_return_val);
				break;
			case IS_OBJECT:
				// If the return value is an object, will assume this is only a reference
				break;
			case IS_STRING:
				EnvSetpType(pv_env, pdo_return_val, STRING);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, strdup(Z_STRVAL_P(&zv_php_ret_val))));
				break;
			case IS_NULL:
				EnvSetpType(pv_env, pdo_return_val, SYMBOL);
				EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
				break;
			default:
				EnvSetpType(pv_env, pdo_return_val, SYMBOL);
				EnvSetpValue(pv_env, pdo_return_val, EnvFalseSymbol(pv_env));
		}
	}
	else {
			EnvSetpType(pv_env, pdo_return_val, SYMBOL);
			EnvSetpValue(pv_env, pdo_return_val, EnvFalseSymbol(pv_env));
	}

	// Destroy all the php parameter variables
	for(i = 0; i < i_argc; i++) {
		if(i_types[i] == INSTANCE_NAME
			|| i_types[i] == INSTANCE_ADDRESS) {
			efree(ppzv_params[i]);
		}
		else
			zval_ptr_dtor(ppzv_params[i]);
	}

	efree(i_types);
	efree(ppzv_params);
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
		EnvSetpType(pv_env, pdo_return_val, SYMBOL);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
		return ;
	}

	// Test if the first argument is string(function name)
	DATA_OBJECT do_php_function;
	if(!EnvArgTypeCheck(pv_env, "php_call", 1, STRING, &do_php_function)) {
		EnvSetpType(pv_env, pdo_return_val, SYMBOL);
		EnvSetpValue(pv_env, pdo_return_val, EnvAddSymbol(pv_env, "nil"));
		return ;
	}

	call_php_function(NULL, DOToString(do_php_function), pdo_return_val, pv_env, 2, EnvRtnArgCount(pv_env) - 1);
}
