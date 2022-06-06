//
// Created by tholz on 06.06.2022.
//

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "json_parse.h"

#define MAX_NESTING_LEVEL		1000

typedef enum {
	JSON_PARSE_STATE_INIT,
	JSON_PARSE_STATE_OBJECT_START,
	JSON_PARSE_STATE_OBJECT_KEY,
	JSON_PARSE_STATE_NAME_VAL_DELIM,
	JSON_PARSE_STATE_OBJECT_VALUE,
	JSON_PARSE_STATE_OBJECT_VALUE_ARRAY,
	JSON_PARSE_STATE_OBJECT_END,
	JSON_PARSE_STATE_ERROR,
} json_parse_state_t;

typedef enum {
	JSON_PARSE_ERRCODE_OK,
	JSON_PARSE_ERRCODE_EXPECTED_OBJECT_START,
	JSON_PARSE_ERRCODE_EXPECTED_OBJECT_KEY,
	JSON_PARSE_ERRCODE_EXPECTED_NAME_VAL_DELIM,
	JSON_PARSE_ERRCODE_EXPECTED_OBJECT_VALUE,
	JSON_PARSE_ERRCODE_MALLOC,
} json_parse_errcode_t;

typedef struct {
	json_parse_errcode_t code;
	json_token_type_t type;
} json_parse_error_t;

static struct {
	json_parse_state_t state;
	json_object_t *root;
	json_object_t *current;
	uint32_t nesting_level;
	json_parse_error_t error;
} m_json_parse;

static json_ret_code_t json_parse_object_token(json_token_t* p_token);

static json_parse_state_t json_parse_state_init(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_start(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_key(json_token_t *p_token);
static json_parse_state_t json_parse_state_name_val_delim(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_value(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_value_array(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_end(json_token_t *p_token);

json_ret_code_t json_parse_object(json_token_t* tokens, uint32_t num_tokens, json_object_t* p_object) {
	uint32_t tokens_consumed = 0;
	m_json_parse.root = p_object;
	m_json_parse.current = p_object;

	while (tokens_consumed < num_tokens) {
		json_ret_code_t ret = json_parse_object_token(&tokens[tokens_consumed]);
		if (ret != JSON_RETVAL_OK && ret != JSON_RETVAL_BUSY) {
			printf("Failed to parse object: %d, errcode is %d, error type is %d\n", ret, m_json_parse.error.code, m_json_parse.error.type);
			return ret;
		}
		tokens_consumed++;
	}

	return JSON_RETVAL_FAIL;
}

static json_ret_code_t json_parse_object_token(json_token_t* p_token) {
	assert(p_token != NULL);
	switch (m_json_parse.state) {
		case JSON_PARSE_STATE_INIT:
			m_json_parse.state = json_parse_state_init(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_START:
			m_json_parse.state = json_parse_state_object_start(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_KEY:
			m_json_parse.state = json_parse_state_object_key(p_token);
			break;
		case JSON_PARSE_STATE_NAME_VAL_DELIM:
			m_json_parse.state = json_parse_state_name_val_delim(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_VALUE:
			m_json_parse.state = json_parse_state_object_value(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_VALUE_ARRAY:
			m_json_parse.state = json_parse_state_object_value_array(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_END:
			m_json_parse.state = json_parse_state_object_end(p_token);
			break;
		case JSON_PARSE_STATE_ERROR:
			return JSON_RETVAL_FAIL;
		default:
			printf("Unhandled state %d\n", m_json_parse.state);
			return JSON_RETVAL_FAIL;
	}
	return JSON_RETVAL_BUSY;
}

static json_parse_state_t json_parse_state_init(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_START_OBJECT) {
		return JSON_PARSE_STATE_OBJECT_START;
	}
	m_json_parse.error.code = JSON_PARSE_ERRCODE_EXPECTED_OBJECT_START;
	m_json_parse.error.type = p_token->type;
	return JSON_PARSE_STATE_ERROR;
}

static json_parse_state_t json_parse_state_object_start(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_VAL_STRING) {
		// TODO: Nesting level
		if (m_json_parse.current == NULL) {
			m_json_parse.current = malloc(sizeof(json_object_t));
			if (m_json_parse.current == NULL) {
				m_json_parse.error.code = JSON_PARSE_ERRCODE_MALLOC;
				return JSON_PARSE_STATE_ERROR;
			}
			m_json_parse.current->type = JSON_VALUE_TYPE_OBJECT;
		}
		m_json_parse.current->key = malloc(p_token->value.string.length + 1);
		if (m_json_parse.current->key == NULL) {
			m_json_parse.error.code = JSON_PARSE_ERRCODE_MALLOC;
			return JSON_PARSE_STATE_ERROR;
		}
		memcpy(m_json_parse.current->key, p_token->value.string.data, p_token->value.string.length);
		m_json_parse.current->key[p_token->value.string.length] = '\0';
		printf(" %s\n", m_json_parse.current->key);
		return JSON_PARSE_STATE_OBJECT_KEY;
	}
	m_json_parse.error.code = JSON_PARSE_ERRCODE_EXPECTED_OBJECT_KEY;
	m_json_parse.error.type = p_token->type;
	return JSON_PARSE_STATE_ERROR;
}

static json_parse_state_t json_parse_state_object_key(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_NAME_VAL_DELIM) {
		return JSON_PARSE_STATE_NAME_VAL_DELIM;
	}
	m_json_parse.error.code = JSON_PARSE_ERRCODE_EXPECTED_NAME_VAL_DELIM;
	m_json_parse.error.type = p_token->type;
	return JSON_PARSE_STATE_ERROR;
}

static json_parse_state_t json_parse_state_name_val_delim(json_token_t *p_token) {
	switch (p_token->type) {
		case JSON_TOKEN_TYPE_VAL_NULL:
			m_json_parse.current->type = JSON_VALUE_TYPE_NULL;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_BOOLEAN:
			m_json_parse.current->type = JSON_VALUE_TYPE_BOOLEAN;
			m_json_parse.current->value.boolean = p_token->value.boolean;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_NUMBER:
			m_json_parse.current->type = JSON_VALUE_TYPE_NUMBER;
			m_json_parse.current->value.number = p_token->value.number;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_STRING:
			m_json_parse.current->type = JSON_VALUE_TYPE_STRING;
			m_json_parse.current->value.string = malloc(p_token->value.string.length + 1);
			if (m_json_parse.current->value.string == NULL) {
				m_json_parse.error.code = JSON_PARSE_ERRCODE_MALLOC;
				return JSON_PARSE_STATE_ERROR;
			}
			memcpy(m_json_parse.current->value.string, p_token->value.string.data, p_token->value.string.length);
			m_json_parse.current->value.string[p_token->value.string.length] = '\0';
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_START_ARRAY:
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY;
		case JSON_TOKEN_TYPE_START_OBJECT:
			return JSON_PARSE_STATE_OBJECT_START;
		default:
			m_json_parse.error.code = JSON_PARSE_ERRCODE_EXPECTED_OBJECT_VALUE;
			m_json_parse.error.type = p_token->type;
			return JSON_PARSE_STATE_ERROR;
	}
}

static json_parse_state_t json_parse_state_object_value(json_token_t *p_token) {
	return JSON_PARSE_STATE_ERROR;
}

static json_parse_state_t json_parse_state_object_value_array(json_token_t *p_token) {
	return JSON_PARSE_STATE_ERROR;
}

static json_parse_state_t json_parse_state_object_end(json_token_t *p_token) {
	return JSON_PARSE_STATE_ERROR;
}
