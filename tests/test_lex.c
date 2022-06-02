//
// Created by tholz on 02.06.2022.
//

#include <string.h>
#include "test_lex.h"
#include "testlib.h"
#include "../json_lex.h"

static inline json_ret_code_t json_strcmp_partial(const char* expect_str, const char* actual_str,
												  uint16_t expect_str_len, uint16_t actual_str_len) {
	if (expect_str_len < actual_str_len) {
		return JSON_RETVAL_FAIL;
	}
	if (expect_str_len == actual_str_len) {
		return strcmp(expect_str, actual_str) == 0 ? JSON_RETVAL_OK : JSON_RETVAL_FAIL;
	}
	return strncmp(expect_str, actual_str, actual_str_len) == 0 ? JSON_RETVAL_INCOMPLETE : JSON_RETVAL_FAIL;
}

TEST_DEF(test_json_lex, lex_complete) {
	TEST_READ_FILE(buffer, "tests/files/complete.json");
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	if (json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len) != JSON_RETVAL_OK) {
		printf("Error lexing!\n");
		return 1;
	}

	printf("Tokens: ");
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		printf("[%s] ", str);
	}
	printf("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_lex, lex_invalid_value) {
	char *buffer = "{\"key\": invalidValue}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	json_ret_code_t ret = json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len);
	TEST_EXPECT_EQ(ret, JSON_RETVAL_ILLEGAL);

	printf("Tokens: ");
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		printf("[%s] ", str);
	}
	printf("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_strcmp_partial, json_strcmp_partial) {
	char* expected_str = "true";
	char* actual_str = "true";
	TEST_EXPECT_EQ(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_OK);

	expected_str = "true";
	actual_str = "truee";
	TEST_EXPECT_EQ(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_FAIL);

	expected_str = "true";
	actual_str = "tree";
	TEST_EXPECT_EQ(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_FAIL);

	expected_str = "true";
	actual_str = "tru";
	TEST_EXPECT_EQ(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_INCOMPLETE);

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_lex() {
	TEST_REG(test_json_lex, lex_complete);
	TEST_REG(test_json_lex, lex_invalid_value);
	//TEST_REG(test_json_strcmp_partial, json_strcmp_partial);
	TESTS_RUN();
}