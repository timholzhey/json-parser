//
// Created by tholz on 02.06.2022.
//

#include <string.h>
#include "test_json.h"
#include "json/json_lex.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#include "testlib.h"

TEST_DEF(test_json_lex, lex_complete) {
	TEST_READ_FILE(buffer, "tests/files/complete.json");
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	if (json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len) != JSON_RETVAL_OK) {
		log_debug("Error lexing!\n");
		return 1;
	}

	log_raw_trace("Tokens: ");
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		log_trace("[%s] ", str);
	}
	log_raw_trace("\n");

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
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_ILLEGAL);

	log_raw_trace("Tokens: ");
	json_token_type_t expected_token_types[] = {JSON_TOKEN_TYPE_START_OBJECT, JSON_TOKEN_TYPE_VAL_STRING,
												JSON_TOKEN_TYPE_NAME_VAL_DELIM};
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		log_trace("[%s] ", str);
		TEST_ASSERT_EQ_U8(tokens[i].type, expected_token_types[i]);
	}
	log_raw_trace("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_lex, lex_invalid_unicode) {
	char *buffer = "{\"key\": \"stringWithIllegalUnicodeEscapeSequence\\u123x\"}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	json_ret_code_t ret = json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_ILLEGAL);

	log_raw_trace("Tokens: ");
	json_token_type_t expected_token_types[] = {JSON_TOKEN_TYPE_START_OBJECT, JSON_TOKEN_TYPE_VAL_STRING,
												JSON_TOKEN_TYPE_NAME_VAL_DELIM};
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		log_trace("[%s] ", str);
		TEST_ASSERT_EQ_U8(tokens[i].type, expected_token_types[i]);
	}
	log_raw_trace("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_lex, lex_invalid_number) {
	char *buffer = "{\"key\": 1.}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	json_ret_code_t ret = json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_ILLEGAL);

	log_raw_trace("Tokens: ");
	json_token_type_t expected_token_types[] = {JSON_TOKEN_TYPE_START_OBJECT, JSON_TOKEN_TYPE_VAL_STRING,
												JSON_TOKEN_TYPE_NAME_VAL_DELIM};
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		log_raw_trace("[%s] ", str);
		TEST_ASSERT_EQ_U8(tokens[i].type, expected_token_types[i]);
	}
	log_raw_trace("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_lex, lex_string_escape) {
	char *buffer = "{\"key\": \"stringWithEscape\\tSequences\" }";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	// Lex
	uint32_t tokens_len = 1000;
	json_token_t *tokens = malloc(tokens_len * sizeof(json_token_t));
	uint32_t num_tokens = 0;
	json_lex_init();
	json_ret_code_t ret = json_lex(buffer, buffer_size, tokens, &num_tokens, tokens_len);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	log_raw_trace("Tokens: ");
	json_token_type_t expected_token_types[] = {JSON_TOKEN_TYPE_START_OBJECT, JSON_TOKEN_TYPE_VAL_STRING,
												JSON_TOKEN_TYPE_NAME_VAL_DELIM, JSON_TOKEN_TYPE_VAL_STRING,
												JSON_TOKEN_TYPE_END_OBJECT};
	TEST_ASSERT_EQ_U32(num_tokens, 5);
	const char* expected_strings[] = {NULL, "key", NULL, "stringWithEscape\tSequences", NULL};
	
	for (uint32_t i = 0; i < num_tokens; i++) {
		const int str_len = 255;
		char str[str_len];
		json_get_token_str_repr(&tokens[i], str, str_len);
		log_raw_trace("[%s] ", str);
		TEST_ASSERT_EQ_U8(tokens[i].type, expected_token_types[i]);
		if (expected_strings[i] != NULL) {
			TEST_ASSERT_EQ_STRING(tokens[i].value.string.data, expected_strings[i], strlen(expected_strings[i]));
		}
	}
	log_raw_trace("\n");

	// Free tokens
	free(tokens);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_strcmp_partial, json_strcmp_partial) {
	char* expected_str = "true";
	char* actual_str = "true";
	TEST_EXPECT_EQ_U8(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_OK);

	expected_str = "true";
	actual_str = "truee";
	TEST_EXPECT_EQ_U8(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_FAIL);

	expected_str = "true";
	actual_str = "tree";
	TEST_EXPECT_EQ_U8(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_FAIL);

	expected_str = "true";
	actual_str = "tru";
	TEST_EXPECT_EQ_U8(json_strcmp_partial(expected_str, actual_str, strlen(expected_str), strlen(actual_str)), JSON_RETVAL_INCOMPLETE);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_str_unescape, json_str_unescape) {
	char* expected_str = "stringWithTabEscape\tSequences";
	char* actual_str = "stringWithTabEscape\\tSequences";
	char* actual_str_unescaped = malloc(255);
	TEST_EXPECT_EQ_U8(json_str_unescape(actual_str_unescaped, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_ASSERT_EQ_STRING(actual_str_unescaped, expected_str, strlen(expected_str));

	expected_str = "stringWithQuotedEscape\"Sequences";
	actual_str = "stringWithQuotedEscape\\\"Sequences";
	TEST_EXPECT_EQ_U8(json_str_unescape(actual_str_unescaped, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_ASSERT_EQ_STRING(actual_str_unescaped, expected_str, strlen(expected_str));

	actual_str = "stringWithIllegalEscape\\Sequences";
	TEST_EXPECT_EQ_U8(json_str_unescape(actual_str_unescaped, actual_str, strlen(actual_str)), JSON_RETVAL_ILLEGAL);

	actual_str = "stringWithIllegalEscapeSequences\\";
	TEST_EXPECT_EQ_U8(json_str_unescape(actual_str_unescaped, actual_str, strlen(actual_str)), JSON_RETVAL_INCOMPLETE);

	expected_str = "stringWithNewLineEscape\nSequences";
	actual_str = "stringWithNewLineEscape\\nSequences";
	TEST_EXPECT_EQ_U8(json_str_unescape(actual_str_unescaped, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_ASSERT_EQ_STRING(actual_str_unescaped, expected_str, strlen(expected_str));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse_number, json_parse_number) {
	double expected_number = 1.0f;
	char* actual_str = "1";
	double actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0e0";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0e+0";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0e-0";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0e+1";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = 1.0f;
	actual_str = "1.0e-1";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = -1.0f;
	actual_str = "-1";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_EQ_DOUBLE(expected_number, actual_number);

	expected_number = -123.456f;
	actual_str = "-123.456";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_TRUE(expected_number - actual_number < 0.00001f);

	expected_number = -123456.0f;
	actual_str = "-123.456e3";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_OK);
	TEST_EXPECT_TRUE(expected_number - actual_number < 0.00001f);

	actual_str = "x";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_FAIL);

	actual_str = "1.";
	actual_number = 0.0f;
	TEST_EXPECT_EQ_U8(json_parse_number(&actual_number, actual_str, strlen(actual_str)), JSON_RETVAL_INCOMPLETE);

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_lex() {
	TEST_GROUP_REG(test_json_lex);
	TEST_REG(test_json_lex, lex_complete);
	TEST_REG(test_json_lex, lex_invalid_value);
	TEST_REG(test_json_lex, lex_invalid_number);
	TEST_REG(test_json_lex, lex_invalid_unicode);
	TEST_REG(test_json_lex, lex_string_escape);
	TEST_REG(test_json_strcmp_partial, json_strcmp_partial);
	TEST_REG(test_json_str_unescape, json_str_unescape);
	TEST_REG(test_json_parse_number, json_parse_number);
	TESTS_RUN();
}
