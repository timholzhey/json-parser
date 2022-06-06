//
// Created by tholz on 06.06.2022.
//

#include <stdlib.h>
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

	return JSON_RETVAL_FAIL;
}