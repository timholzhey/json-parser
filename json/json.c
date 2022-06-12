//
// Created by tholz on 06.06.2022.
//

#include <stdlib.h>
#include <string.h>
#include "json.h"
#include "json_lex.h"
#include "json_parse.h"
#include "json_stringify.h"

#define MAX_TOKEN_LENGTH		1000

json_ret_code_t json_parse(const char* p_data, size_t size, json_object_t* p_object) {
	// Lex
	json_token_t *tokens = malloc(MAX_TOKEN_LENGTH * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	json_ret_code_t lex_ret = json_lex(p_data, size, tokens, &num_tokens, MAX_TOKEN_LENGTH);
	if (lex_ret != JSON_RETVAL_OK) {
		free(tokens);
		return lex_ret;
	}

	// Parse
	json_ret_code_t parse_ret = json_parse_object(tokens, num_tokens, p_object);
	if (parse_ret != JSON_RETVAL_OK) {
		free(tokens);
		return parse_ret;
	}

	free(tokens);

	return JSON_RETVAL_OK;
}

json_value_t* json_object_get_value(const json_object_t* p_object, const char* key) {
	if (p_object == NULL) {
		return NULL;
	}

	for (uint32_t i = 0; i < p_object->num_members; i++) {
		if (strcmp(p_object->members[i]->key, key) == 0) {
			return &p_object->members[i]->value;
		}
	}

	return NULL;
}

json_value_type_t json_object_get_value_type(const json_object_t* p_object, const char* key) {
	if (p_object == NULL) {
		return JSON_VALUE_TYPE_UNDEFINED;
	}

	for (uint32_t i = 0; i < p_object->num_members; i++) {
		if (strcmp(p_object->members[i]->key, key) == 0) {
			return p_object->members[i]->type;
		}
	}

	return JSON_VALUE_TYPE_UNDEFINED;
}

json_value_t* json_value_get_array_member(json_value_t* p_value, uint32_t index) {
	if (p_value == NULL) {
		return NULL;
	}

	if (index >= p_value->array->length) {
		return NULL;
	}

	return &p_value->array->values[index]->value;
}

bool json_object_has_key(const json_object_t* p_object, const char* key) {
	if (p_object == NULL) {
		return false;
	}

	for (uint32_t i = 0; i < p_object->num_members; i++) {
		if (strcmp(p_object->members[i]->key, key) == 0) {
			return true;
		}
	}

	return false;
}

json_ret_code_t json_object_add_value(json_object_t *p_object, const char* key, json_value_t value, json_value_type_t type) {
	if (p_object == NULL) {
		return JSON_RETVAL_INVALID_PARAM;
	}

	if (p_object->num_members >= JSON_NUM_MEMBERS) {
		return JSON_RETVAL_FAIL;
	}

	p_object->members[p_object->num_members] = malloc(sizeof(json_object_member_t));
	if (p_object->members[p_object->num_members] == NULL) {
		return JSON_RETVAL_FAIL;
	}

	p_object->members[p_object->num_members]->key = malloc(strlen(key) + 1);
	if (p_object->members[p_object->num_members]->key == NULL) {
		return JSON_RETVAL_FAIL;
	}

	strcpy(p_object->members[p_object->num_members]->key, key);
	p_object->members[p_object->num_members]->value = value;
	p_object->members[p_object->num_members]->type = type;
	p_object->num_members++;

	return JSON_RETVAL_OK;
}

json_ret_code_t json_object_free(json_object_t* p_object) {
	if (p_object == NULL) {
		return JSON_RETVAL_INVALID_PARAM;
	}

	for (uint32_t i = 0; i < p_object->num_members; i++) {
		free(p_object->members[i]->key);
		if (p_object->members[i]->type == JSON_VALUE_TYPE_OBJECT) {
			json_object_free(p_object->members[i]->value.object);
		} else if (p_object->members[i]->type == JSON_VALUE_TYPE_ARRAY) {
			free(p_object->members[i]->value.array);
		} else if (p_object->members[i]->type == JSON_VALUE_TYPE_STRING) {
			free(p_object->members[i]->value.string);
		}
		free(p_object->members[i]);
	}

	return JSON_RETVAL_OK;
}

char *json_stringify(const json_object_t* p_object) {
	return json_object_stringify(p_object, false);
}

char *json_stringify_pretty(const json_object_t* p_object) {
	return json_object_stringify(p_object, true);
}
