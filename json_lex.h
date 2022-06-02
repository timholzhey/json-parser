//
// Created by tholz on 29.05.2022.
//

#ifndef JSON_PARSER_JSON_LEX_H
#define JSON_PARSER_JSON_LEX_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	JSON_TOKEN_TYPE_UNDEFINED,
	JSON_TOKEN_TYPE_START_OBJECT,
	JSON_TOKEN_TYPE_END_OBJECT,
	JSON_TOKEN_TYPE_MEMBER_DELIM,
	JSON_TOKEN_TYPE_NAME_VAL_DELIM,
	JSON_TOKEN_TYPE_VAL_START_ARRAY,
	JSON_TOKEN_TYPE_VAL_END_ARRAY,
	JSON_TOKEN_TYPE_VAL_BOOLEAN,
	JSON_TOKEN_TYPE_VAL_NULL,
	JSON_TOKEN_TYPE_VAL_STRING,
	JSON_TOKEN_TYPE_VAL_NUMBER,
	JSON_TOKEN_TYPE_WHITESPACE,
	JSON_TOKEN_TYPE_COUNT,
} json_token_type_t;

typedef enum {
	JSON_RETVAL_OK,
	JSON_RETVAL_INCOMPLETE,
	JSON_RETVAL_BUSY,
	JSON_RETVAL_FAIL,
	JSON_RETVAL_ILLEGAL,
} json_ret_code_t;

typedef json_ret_code_t (*json_is_token_type_fn)();

typedef struct {
	char* name;
	uint8_t flags;
	json_is_token_type_fn is_token_type_fn;
} json_token_type_def_t;

typedef struct {
	char* data;
	uint16_t length;
} json_value_string_t;

typedef union {
	json_value_string_t string;
	bool boolean;
	float number;
} json_value_t;

typedef struct {
	json_token_type_t type;
	json_value_t value;
	uint16_t line;
	uint16_t column;
} json_token_t;

#define JSON_TOKEN_FLAG_NONE		0x00
#define JSON_TOKEN_FLAG_RESERVED	0x01
#define JSON_TOKEN_FLAG_IGNORED		0x02
#define JSON_TOKEN_FLAG_CONTINUOUS	0x04

#define JSON_IS_TOKEN_TYPE_FN_NAME(token_type) json_is_token_type_ ## token_type
#define JSON_IS_TOKEN_TYPE_FN_DECL(token_type) json_ret_code_t JSON_IS_TOKEN_TYPE_FN_NAME(token_type)();
#define JSON_IS_TOKEN_TYPE_FN_IMPL(token_type) json_ret_code_t JSON_IS_TOKEN_TYPE_FN_NAME(token_type)()

#define JSON_RETURN_BOOL(boolean) return (boolean) ? JSON_RETVAL_OK : JSON_RETVAL_FAIL

#define JSON_TOKEN_TYPE_DEF(type, _name, _flags) [type] = { \
    .name = (_name), \
    .flags = (_flags), \
    .is_token_type_fn = JSON_IS_TOKEN_TYPE_FN_NAME(type), \
}

JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_START_OBJECT)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_END_OBJECT)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_MEMBER_DELIM)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_NAME_VAL_DELIM)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_START_ARRAY)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_END_ARRAY)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_BOOLEAN)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_NULL)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_STRING)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_VAL_NUMBER)
JSON_IS_TOKEN_TYPE_FN_DECL(JSON_TOKEN_TYPE_WHITESPACE)

#ifndef MIN
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif

#ifndef MAX
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#endif

void json_lex_init();

json_ret_code_t json_lex(const char* p_input, uint32_t input_len, json_token_t* p_tokens, uint32_t *p_num_tokens, uint32_t max_num_tokens);

char* json_get_token_name(json_token_type_t token_type);

void json_get_token_str_repr(json_token_t* p_token, char* str, uint32_t str_len);

#endif //JSON_PARSER_JSON_LEX_H
