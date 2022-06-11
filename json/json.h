//
// Created by tholz on 06.06.2022.
//

#ifndef JSON_PARSER_JSON_H
#define JSON_PARSER_JSON_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum {
	JSON_RETVAL_OK,
	JSON_RETVAL_INCOMPLETE,
	JSON_RETVAL_BUSY,
	JSON_RETVAL_FAIL,
	JSON_RETVAL_ILLEGAL,
	JSON_RETVAL_FINISHED,
	JSON_RETVAL_INVALID_PARAM,
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

typedef union json_value_t json_value_t;
typedef struct json_object_t json_object_t;
typedef struct json_array_t json_array_t;

union json_value_t {
	char* string;
	double number;
	bool boolean;
	json_array_t* array;
	struct json_object_t* object;
};

typedef struct {
	json_value_t value;
	json_value_type_t type;
} json_array_member_t;

struct json_array_t {
	json_array_member_t* values[10000];
	size_t length;
};

typedef struct {
	char* key;
	json_value_t value;
	json_value_type_t type;
} json_object_member_t;

struct json_object_t {
	json_object_member_t* members[10000];
	uint32_t num_members;
	struct json_object_t* parent;
};

#define json_parse_string(string, name) \
	json_object_t name; \
	json_ret_code_t name ## _return = json_parse(string, strlen(string), &(name));

json_ret_code_t json_parse(const char* p_data, size_t size, json_object_t* p_object);

json_value_t* json_object_get_value(const json_object_t* p_object, const char* key);
json_value_type_t json_object_get_value_type(const json_object_t* p_object, const char* key);
json_value_t* json_value_get_array_member(json_value_t* p_value, uint32_t index);
bool json_object_has_key(const json_object_t* p_object, const char* key);

#endif //JSON_PARSER_JSON_H
