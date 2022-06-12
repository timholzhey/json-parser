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
	JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM,
	JSON_PARSE_STATE_OBJECT_END,
	JSON_PARSE_STATE_MEMBER_DELIM,
	JSON_PARSE_STATE_END,
	JSON_PARSE_STATE_ERROR,
} json_parse_state_t;

static struct {
	json_parse_state_t state;
	json_object_t *root;
	json_object_t *current;
	int32_t nesting_level;
	bool is_array;
} m_json_parse;

static json_ret_code_t json_parse_object_token(json_token_t* p_token);

static json_parse_state_t json_parse_state_init(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_start(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_key(json_token_t *p_token);
static json_parse_state_t json_parse_state_name_val_delim(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_value(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_value_array(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_value_array_delim(json_token_t *p_token);
static json_parse_state_t json_parse_state_object_end(json_token_t *p_token);
static json_parse_state_t json_parse_state_member_delim(json_token_t *p_token);
static json_parse_state_t json_parse_state_end(json_token_t *p_token);

json_ret_code_t json_parse_object(json_token_t* tokens, uint32_t num_tokens, json_object_t* p_object) {
	uint32_t tokens_consumed = 0;
	if (p_object == NULL) {
		return JSON_RETVAL_INVALID_PARAM;
	}
	memset(&m_json_parse, 0, sizeof(m_json_parse));
	memset(p_object, 0, sizeof(json_object_t));
	m_json_parse.root = p_object;
	m_json_parse.current = p_object;

	while (tokens_consumed < num_tokens) {
		json_ret_code_t ret = json_parse_object_token(&tokens[tokens_consumed]);
		if (ret != JSON_RETVAL_BUSY) {
			return ret;
		}
		tokens_consumed++;
	}

	if (m_json_parse.nesting_level != 0) {
		printf("\033[31mFailed to parse object: Unclosed object\033[0m\n");
		return JSON_RETVAL_FAIL;
	}

	return JSON_RETVAL_OK;
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
		case JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM:
			m_json_parse.state = json_parse_state_object_value_array_delim(p_token);
			break;
		case JSON_PARSE_STATE_OBJECT_END:
			m_json_parse.state = json_parse_state_object_end(p_token);
			break;
		case JSON_PARSE_STATE_MEMBER_DELIM:
			m_json_parse.state = json_parse_state_member_delim(p_token);
			break;
		case JSON_PARSE_STATE_END:
			m_json_parse.state = json_parse_state_end(p_token);
			break;
		case JSON_PARSE_STATE_ERROR:
			return JSON_RETVAL_FAIL;
		default:
			printf("Unhandled state %d\n", m_json_parse.state);
			return JSON_RETVAL_FAIL;
	}
	if (m_json_parse.state == JSON_PARSE_STATE_ERROR) {
		return JSON_RETVAL_FAIL;
	}
	return JSON_RETVAL_BUSY;
}

#define JSON_PARSER_REPORT_ERROR(msg, ...) { \
	printf("\033[31mFailed to parse object: "); \
	printf(msg, ##__VA_ARGS__);                 \
	printf(" at %u:%u\033[0m\n", p_token->line, p_token->column); \
	return JSON_PARSE_STATE_ERROR;              \
}

static json_parse_state_t json_parse_state_init(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_START_OBJECT) {
		if (m_json_parse.nesting_level + 1 >= MAX_NESTING_LEVEL) {
			JSON_PARSER_REPORT_ERROR("Maximum nesting level (%u) exceeded", MAX_NESTING_LEVEL);
		}
		m_json_parse.nesting_level++;
		return JSON_PARSE_STATE_OBJECT_START;
	}
	JSON_PARSER_REPORT_ERROR("Expected object start, but got %s", json_get_token_name(p_token->type));
}

#define JSON_PARSE_HANDLE_MALLOC(not_null) \
	if ((not_null) == NULL) { \
		JSON_PARSER_REPORT_ERROR("Failed to allocate memory"); \
		return JSON_PARSE_STATE_ERROR; \
	}

static json_parse_state_t json_parse_state_object_start(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_VAL_STRING) {
		JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members] = calloc(1, sizeof(json_object_member_t)));
		JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->key = malloc(p_token->value.string.length + 1));
		memcpy(m_json_parse.current->members[m_json_parse.current->num_members]->key, p_token->value.string.data, p_token->value.string.length);
		m_json_parse.current->members[m_json_parse.current->num_members]->key[p_token->value.string.length] = '\0';

		return JSON_PARSE_STATE_OBJECT_KEY;
	}
	if (p_token->type == JSON_TOKEN_TYPE_END_OBJECT) {
		m_json_parse.nesting_level--;
		if (m_json_parse.nesting_level <= 0) {
			return JSON_PARSE_STATE_END;
		}
		return JSON_PARSE_STATE_OBJECT_END;
	}
	JSON_PARSER_REPORT_ERROR("Expected object key or object end, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_object_key(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_NAME_VAL_DELIM) {
		return JSON_PARSE_STATE_NAME_VAL_DELIM;
	}
	JSON_PARSER_REPORT_ERROR("Expected name value delimiter, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_name_val_delim(json_token_t *p_token) {
	switch (p_token->type) {
		case JSON_TOKEN_TYPE_VAL_NULL:
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_NULL;
			m_json_parse.current->num_members++;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_BOOLEAN:
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_BOOLEAN;
			m_json_parse.current->members[m_json_parse.current->num_members]->value.boolean = p_token->value.boolean;
			m_json_parse.current->num_members++;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_NUMBER:
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_NUMBER;
			m_json_parse.current->members[m_json_parse.current->num_members]->value.number = p_token->value.number;
			m_json_parse.current->num_members++;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_STRING:
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_STRING;
			JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->value.string = malloc(p_token->value.string.length + 1));
			memcpy(m_json_parse.current->members[m_json_parse.current->num_members]->value.string, p_token->value.string.data, p_token->value.string.length);
			m_json_parse.current->members[m_json_parse.current->num_members]->value.string[p_token->value.string.length] = '\0';
			m_json_parse.current->num_members++;
			return JSON_PARSE_STATE_OBJECT_VALUE;
		case JSON_TOKEN_TYPE_VAL_START_ARRAY:
			m_json_parse.is_array = true;
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_ARRAY;
			JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->value.array = calloc(1, sizeof(json_array_t)));
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY;
		case JSON_TOKEN_TYPE_START_OBJECT:
			m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_OBJECT;
			JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->value.object = calloc(1, sizeof(json_object_t)));
			m_json_parse.current->members[m_json_parse.current->num_members]->value.object->parent = m_json_parse.current;
			m_json_parse.current = m_json_parse.current->members[m_json_parse.current->num_members]->value.object;
			if (m_json_parse.nesting_level + 1 >= MAX_NESTING_LEVEL) {
				JSON_PARSER_REPORT_ERROR("Maximum nesting level (%u) exceeded", MAX_NESTING_LEVEL);
			}
			m_json_parse.nesting_level++;
			return JSON_PARSE_STATE_OBJECT_START;
		default:
			JSON_PARSER_REPORT_ERROR("Expected value, but got %s", json_get_token_name(p_token->type));
	}
}

static json_parse_state_t json_parse_state_object_value(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_MEMBER_DELIM) {
		return JSON_PARSE_STATE_MEMBER_DELIM;
	}
	if (p_token->type == JSON_TOKEN_TYPE_END_OBJECT) {
		m_json_parse.nesting_level--;
		if (m_json_parse.nesting_level <= 0) {
			return JSON_PARSE_STATE_END;
		}
		return JSON_PARSE_STATE_OBJECT_END;
	}
	JSON_PARSER_REPORT_ERROR("Expected member delimiter or object end, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_member_delim(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_VAL_STRING) {
		JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members] = calloc(1, sizeof(json_object_member_t)));
		JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->key = malloc(p_token->value.string.length + 1));
		memcpy(m_json_parse.current->members[m_json_parse.current->num_members]->key, p_token->value.string.data, p_token->value.string.length);
		m_json_parse.current->members[m_json_parse.current->num_members]->key[p_token->value.string.length] = '\0';

		return JSON_PARSE_STATE_OBJECT_KEY;
	}
	if (p_token->type == JSON_TOKEN_TYPE_START_OBJECT) { // TODO: ?
		m_json_parse.current->members[m_json_parse.current->num_members]->type = JSON_VALUE_TYPE_OBJECT;
		JSON_PARSE_HANDLE_MALLOC(m_json_parse.current->members[m_json_parse.current->num_members]->value.object = calloc(1, sizeof(json_object_t)));
		m_json_parse.current->members[m_json_parse.current->num_members]->value.object->parent = m_json_parse.current;
		m_json_parse.current = m_json_parse.current->members[m_json_parse.current->num_members]->value.object;
		m_json_parse.nesting_level++;
		return JSON_PARSE_STATE_OBJECT_START;
	}
	JSON_PARSER_REPORT_ERROR("Expected object key or object start, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_object_value_array(json_token_t *p_token) {
	json_array_t* p_array = m_json_parse.current->members[m_json_parse.current->num_members]->value.array;
	switch (p_token->type) {
		case JSON_TOKEN_TYPE_VAL_NULL:
			JSON_PARSE_HANDLE_MALLOC(p_array->values[p_array->length] = malloc(sizeof(json_value_t)));
			p_array->values[p_array->length++]->type = JSON_VALUE_TYPE_NULL;
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM;
		case JSON_TOKEN_TYPE_VAL_BOOLEAN:
			JSON_PARSE_HANDLE_MALLOC(p_array->values[p_array->length] = malloc(sizeof(json_value_t)));
			p_array->values[p_array->length]->type = JSON_VALUE_TYPE_BOOLEAN;
			p_array->values[p_array->length++]->value.boolean = p_token->value.boolean;
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM;
		case JSON_TOKEN_TYPE_VAL_NUMBER:
			JSON_PARSE_HANDLE_MALLOC(p_array->values[p_array->length] = malloc(sizeof(json_value_t)));
			p_array->values[p_array->length]->type = JSON_VALUE_TYPE_NUMBER;
			p_array->values[p_array->length++]->value.number = p_token->value.number;
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM;
		case JSON_TOKEN_TYPE_VAL_STRING:
			JSON_PARSE_HANDLE_MALLOC(p_array->values[p_array->length] = malloc(sizeof(json_value_t)));
			p_array->values[p_array->length]->type = JSON_VALUE_TYPE_STRING;
			JSON_PARSE_HANDLE_MALLOC(p_array->values[p_array->length]->value.string = malloc(p_token->value.string.length + 1));
			memcpy(p_array->values[p_array->length]->value.string, p_token->value.string.data, p_token->value.string.length);
			p_array->values[p_array->length++]->value.string[p_token->value.string.length] = '\0';
			return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY_DELIM;
		default:
			JSON_PARSER_REPORT_ERROR("Expected value, but got %s", json_get_token_name(p_token->type));
	}
}

static json_parse_state_t json_parse_state_object_value_array_delim(json_token_t *p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_MEMBER_DELIM) {
		return JSON_PARSE_STATE_OBJECT_VALUE_ARRAY;
	}
	if (p_token->type == JSON_TOKEN_TYPE_VAL_END_ARRAY) {
		m_json_parse.is_array = false;
		m_json_parse.current->num_members++;
		return JSON_PARSE_STATE_OBJECT_VALUE;
	}
	JSON_PARSER_REPORT_ERROR("Expected value delimiter, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_object_end(json_token_t *p_token) {
	if (m_json_parse.nesting_level <= 0) {
		return JSON_PARSE_STATE_END;
	}
	m_json_parse.current = m_json_parse.current->parent;
	m_json_parse.current->num_members++;

	if (p_token->type == JSON_TOKEN_TYPE_MEMBER_DELIM) {
		return JSON_PARSE_STATE_MEMBER_DELIM;
	}

	m_json_parse.nesting_level--;
	if (m_json_parse.nesting_level <= 0) {
		return JSON_PARSE_STATE_END;
	}

	if (p_token->type == JSON_TOKEN_TYPE_END_OBJECT) {
		return JSON_PARSE_STATE_OBJECT_END;
	}
	JSON_PARSER_REPORT_ERROR("Expected member delimiter or object end, but got %s", json_get_token_name(p_token->type));
}

static json_parse_state_t json_parse_state_end(json_token_t *p_token) {
	JSON_PARSER_REPORT_ERROR("Unexpected token `%s` after end of object", json_get_token_name(p_token->type));
}