//
// Created by tholz on 06.06.2022.
//

#ifndef JSON_PARSER_JSON_H
#define JSON_PARSER_JSON_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
	JSON_RETVAL_OK,
	JSON_RETVAL_INCOMPLETE,
	JSON_RETVAL_BUSY,
	JSON_RETVAL_FAIL,
	JSON_RETVAL_ILLEGAL,
	JSON_RETVAL_FINISHED,
} json_ret_code_t;

typedef enum {
	JSON_VALUE_TYPE_UNDEFINED,
	JSON_VALUE_TYPE_STRING,
	JSON_VALUE_TYPE_NUMBER,
	JSON_VALUE_TYPE_BOOLEAN,
	JSON_VALUE_TYPE_NULL,
	JSON_VALUE_TYPE_ARRAY,
	JSON_VALUE_TYPE_OBJECT,
} json_value_type_t;

union json_value_t;
struct json_object_t;

typedef struct {
	union json_value_t* values;
	size_t length;
} json_array_t;

typedef union {
	char* string;
	double number;
	bool boolean;
	json_array_t* array;
	struct json_object_t* object;
} json_value_t;

typedef struct {
	char* key;
	json_value_t value;
	json_value_type_t type;
} json_object_t;

json_ret_code_t json_parse(const char* p_data, size_t size, json_object_t* p_object);

#endif //JSON_PARSER_JSON_H
