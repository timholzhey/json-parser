//
// Created by tholz on 06.06.2022.
//

#include <stdlib.h>
#include <string.h>
#include "json.h"
#include "json_lex.h"
#include "json_parse.h"

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
