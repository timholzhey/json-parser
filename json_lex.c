//
// Created by tholz on 29.05.2022.
//

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "json_lex.h"

#ifndef DEBUG
#define DEBUG	1
#endif

#define JSON_TOKEN_STR_REPR_START_OBJECT				"{"
#define JSON_TOKEN_STR_REPR_END_OBJECT					"}"
#define JSON_TOKEN_STR_REPR_MEMBER_DELIM				","
#define JSON_TOKEN_STR_REPR_NAME_VAL_DELIM				":"
#define JSON_TOKEN_STR_REPR_VAL_START_ARRAY				"["
#define JSON_TOKEN_STR_REPR_VAL_END_ARRAY				"]"
#define JSON_TOKEN_STR_REPR_VAL_BOOLEAN_TRUE			"true"
#define JSON_TOKEN_STR_REPR_VAL_BOOLEAN_FALSE			"false"
#define JSON_TOKEN_STR_REPR_VAL_NULL					"null"
#define JSON_TOKEN_STR_REPR_VAL_STRING_QUOTES			"\""
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC				"\\"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_QUOTE		"\""
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSLASH	"\\"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_SLASH		"/"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSPACE	"b"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_FORMFEED		"f"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_LINEFEED		"n"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_CARRIAGE_RET	"r"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_HORIZ_TAB	"t"
#define JSON_TOKEN_STR_REPR_VAL_STRING_ESC_UNICODE		"u"
#define JSON_TOKEN_STR_REPR_VAL_NUMBER_SIGN_POS			"+"
#define JSON_TOKEN_STR_REPR_VAL_NUMBER_SIGN_NEG			"-"
#define JSON_TOKEN_STR_REPR_VAL_NUMBER_EXPONENT_UPPER	"E"
#define JSON_TOKEN_STR_REPR_VAL_NUMBER_EXPONENT			"e"
#define JSON_TOKEN_STR_REPR_VAL_NUMBER_FRAC				"."
#define JSON_TOKEN_STR_REPR_WHITESPACE_SPACE			" "
#define JSON_TOKEN_STR_REPR_WHITESPACE_LINEFEED			"\n"
#define JSON_TOKEN_STR_REPR_WHITESPACE_CARRIAGE_RET		"\r"
#define JSON_TOKEN_STR_REPR_WHITESPACE_HORIZ_TAB		"\t"
#define JSON_TOKEN_STR_REPR_WHITESPACE_CRLF				"\r\n"

#define JSON_LEX_CHAR_BUFFER_SIZE	UINT16_MAX

static struct {
	char buffer[JSON_LEX_CHAR_BUFFER_SIZE];
	uint16_t buffer_len;
	json_token_t token;
	json_ret_code_t ret_code;
	uint16_t line;
	uint16_t column;
	uint16_t line_start;
} m_json_lex;

static inline json_ret_code_t json_strcmp_partial(const char* expect_str, const char* actual_str,
												  uint16_t expect_str_len, uint16_t actual_str_len) {
	if (expect_str_len < actual_str_len || actual_str_len == 0) {
		return JSON_RETVAL_FAIL;
	}
	if (expect_str_len == actual_str_len) {
		return strcmp(expect_str, actual_str) == 0 ? JSON_RETVAL_OK : JSON_RETVAL_FAIL;
	}
	return strncmp(expect_str, actual_str, actual_str_len) == 0 ? JSON_RETVAL_INCOMPLETE : JSON_RETVAL_FAIL;
}

#define JSON_RETURN_COND_FALSE(c)		if (!(c)) { JSON_RETURN_BOOL(false); }

#define JSON_MATCH_CHAR(c)					(m_json_lex.buffer[0] == (c))
#define JSON_MATCH_STRING(s)				(strncmp(m_json_lex.buffer, (s), MAX(m_json_lex.buffer_len + 1, strlen(s))) == 0)
#define JSON_MATCH_STRING_PARTIAL(s)		(json_strcmp_partial((s), m_json_lex.buffer, strlen(s), m_json_lex.buffer_len + 1))
#define JSON_RETURN_MATCH_STRING(s)			JSON_RETURN_BOOL(JSON_MATCH_STRING(s))
#define JSON_RETURN_MATCH_STRING_PARTIAL(s)	return JSON_MATCH_STRING_PARTIAL(s)

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_START_OBJECT) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_START_OBJECT); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_END_OBJECT) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_END_OBJECT); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_MEMBER_DELIM) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_MEMBER_DELIM); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_NAME_VAL_DELIM) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_NAME_VAL_DELIM); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_START_ARRAY) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_VAL_START_ARRAY); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_END_ARRAY) { JSON_RETURN_MATCH_STRING(JSON_TOKEN_STR_REPR_VAL_END_ARRAY); }
JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_NULL) { JSON_RETURN_MATCH_STRING_PARTIAL(JSON_TOKEN_STR_REPR_VAL_NULL); }

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_BOOLEAN) {
	json_ret_code_t ret;
	if ((ret = JSON_MATCH_STRING_PARTIAL(JSON_TOKEN_STR_REPR_VAL_BOOLEAN_TRUE)) <= JSON_RETVAL_INCOMPLETE) {
		m_json_lex.token.value.boolean = true;
		return ret;
	}
	if ((ret = JSON_MATCH_STRING_PARTIAL(JSON_TOKEN_STR_REPR_VAL_BOOLEAN_FALSE)) <= JSON_RETVAL_INCOMPLETE) {
		m_json_lex.token.value.boolean = false;
		return ret;
	}
	JSON_RETURN_BOOL(false);
}

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_STRING) {
	bool match = true;
	JSON_RETURN_COND_FALSE(JSON_MATCH_CHAR(*JSON_TOKEN_STR_REPR_VAL_STRING_QUOTES));
	for (uint16_t i = 1; i < m_json_lex.buffer_len; i++) {
		if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSLASH) {
			// TODO
		} else if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_QUOTE) {
			JSON_RETURN_BOOL(false);
		} else {
			continue;
		}
	}
	if (m_json_lex.buffer_len >= 2 && m_json_lex.buffer[m_json_lex.buffer_len] == *JSON_TOKEN_STR_REPR_VAL_STRING_QUOTES) {
		m_json_lex.token.value.string.data = calloc(m_json_lex.buffer_len, sizeof(char));
		memcpy(m_json_lex.token.value.string.data, m_json_lex.buffer + 1, m_json_lex.buffer_len - 1);
		m_json_lex.token.value.string.length = m_json_lex.buffer_len - 1;
	} else {
		return JSON_RETVAL_INCOMPLETE;
	}
	JSON_RETURN_BOOL(match);
}

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_NUMBER) {
	return JSON_RETVAL_FAIL; // TODO
}

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_WHITESPACE) {
	JSON_RETURN_BOOL(
		JSON_MATCH_STRING(JSON_TOKEN_STR_REPR_WHITESPACE_SPACE) ||
		JSON_MATCH_STRING(JSON_TOKEN_STR_REPR_WHITESPACE_LINEFEED) ||
		JSON_MATCH_STRING(JSON_TOKEN_STR_REPR_WHITESPACE_CARRIAGE_RET) ||
		JSON_MATCH_STRING(JSON_TOKEN_STR_REPR_WHITESPACE_HORIZ_TAB) ||
		JSON_MATCH_STRING(JSON_TOKEN_STR_REPR_WHITESPACE_CRLF)
		);
}

static json_token_type_def_t json_token_type_def[] = {
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_START_OBJECT,
				"Start object",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_END_OBJECT,
				"End object",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_MEMBER_DELIM,
				"Member delimiter",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_NAME_VAL_DELIM,
				"Name-value delimiter",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_START_ARRAY,
				"Start array",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_END_ARRAY,
				"End array",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_BOOLEAN,
				"Boolean",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_NULL,
				"Null",
				JSON_TOKEN_FLAG_RESERVED
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_STRING,
				"String",
				JSON_TOKEN_FLAG_CONTINUOUS
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_VAL_NUMBER,
				"Number",
				JSON_TOKEN_FLAG_NONE
		),
		JSON_TOKEN_TYPE_DEF(
				JSON_TOKEN_TYPE_WHITESPACE,
				"Whitespace",
				JSON_TOKEN_FLAG_IGNORED
		),
};

static json_ret_code_t get_next_token(const char* p_input, uint32_t input_len, uint32_t* p_consumed, json_token_t* p_token);

void json_lex_init() {
	memset(&m_json_lex, 0, sizeof(m_json_lex));
	m_json_lex.ret_code = JSON_RETVAL_FAIL;
}

static void json_lex_error_handler(const char* p_input, uint32_t input_len, json_ret_code_t ret) {
	if (ret == JSON_RETVAL_ILLEGAL) {
		printf("\033[1;31mSyntaxError\033[0m: Unexpected token \"%.*s\" at position %u:%u\n", m_json_lex.buffer_len + 1, m_json_lex.buffer, m_json_lex.line + 1, m_json_lex.column + 1);
		printf("%5u |     ", m_json_lex.line + 1);
		for (uint16_t i = m_json_lex.line_start; i < input_len && p_input[i] != '\n' && p_input[i] != '\r'; i++) {
			printf("%c", p_input[i]);
		}
		printf("\n");
		printf("      |     ");
		for (uint16_t i = 0; i < m_json_lex.column; i++) {
			printf(" ");
		}
		printf("\033[1;31m^\033[0m\n");
	}
}

json_ret_code_t json_lex(const char* p_input, uint32_t input_len, json_token_t* p_tokens, uint32_t *p_num_tokens, uint32_t max_num_tokens) {
	uint32_t consumed_total = 0;
	m_json_lex.line_start = 0;

	while (*p_num_tokens <= max_num_tokens) {
		json_token_t token = {0};
		uint32_t consumed = 0;
		uint16_t line_num_before = m_json_lex.line;
		json_ret_code_t ret = get_next_token(&p_input[consumed_total], input_len - consumed_total, &consumed, &token);
		if (ret != JSON_RETVAL_OK && ret != JSON_RETVAL_BUSY) {
			json_lex_error_handler(p_input, input_len, ret);
			return ret;
		}
		if (ret == JSON_RETVAL_OK) {
			p_tokens[(*p_num_tokens)++] = token;
		}
		consumed_total += consumed;
		if (m_json_lex.line != line_num_before) {
			m_json_lex.line_start = consumed_total;
		}
	}

	return JSON_RETVAL_OK;
}

static void clear_token(json_token_t* p_token) {
	if (p_token->type == JSON_TOKEN_TYPE_VAL_STRING) {
		free(p_token->value.string.data);
	}
	memset(p_token, 0, sizeof(json_token_t));
}

static void copy_token(json_token_t* p_dest, const json_token_t* p_src) {
	memcpy(p_dest, p_src, sizeof(json_token_t));
	if (p_src->type == JSON_TOKEN_TYPE_VAL_STRING) {
		p_dest->value.string.data = calloc(p_src->value.string.length + 1, sizeof(char));
		memcpy(p_dest->value.string.data, p_src->value.string.data, p_src->value.string.length);
	}
}

static json_ret_code_t get_next_token(const char* p_input, uint32_t input_len, uint32_t* p_consumed, json_token_t* p_token) {
	while (*p_consumed < input_len) {
		m_json_lex.buffer[m_json_lex.buffer_len] = p_input[(*p_consumed)++];
		uint8_t num_matches = 0;

		for (json_token_type_t token_type = 1; token_type < JSON_TOKEN_TYPE_COUNT; token_type++) {
			assert(json_token_type_def[token_type].is_token_type_fn != NULL);
			json_ret_code_t ret = json_token_type_def[token_type].is_token_type_fn();

			if (ret == JSON_RETVAL_OK || ret == JSON_RETVAL_INCOMPLETE) {
				m_json_lex.token.type = token_type;
				m_json_lex.ret_code = ret;
				num_matches++;
			}
		}

		if (num_matches == 0) {
			m_json_lex.column += m_json_lex.buffer_len;
			if (m_json_lex.buffer[0] == '\n' || m_json_lex.buffer[0] == '\r') {
				m_json_lex.line++;
				m_json_lex.column = 0;
			}
			if (m_json_lex.ret_code == JSON_RETVAL_FAIL) {
				return JSON_RETVAL_ILLEGAL;
			}
			bool return_token = false;
			if (m_json_lex.token.type != JSON_TOKEN_TYPE_UNDEFINED) {
				if ((json_token_type_def[m_json_lex.token.type].flags & JSON_TOKEN_FLAG_IGNORED) == 0) {
					copy_token(p_token, &m_json_lex.token);
					return_token = true;
				}
			}
			clear_token(&m_json_lex.token);
			m_json_lex.buffer_len = 0;
			memset(m_json_lex.buffer, 0, sizeof(m_json_lex.buffer));
			m_json_lex.ret_code = JSON_RETVAL_FAIL;
			m_json_lex.token.line = m_json_lex.line;
			m_json_lex.token.column = m_json_lex.column;
			(*p_consumed)--;
			if (return_token) {
				return JSON_RETVAL_OK;
			}
			return JSON_RETVAL_BUSY;
		} else {
			m_json_lex.buffer_len++;
		}
	}

	return JSON_RETVAL_FAIL;
}

char* json_get_token_name(json_token_type_t token_type) {
	return json_token_type_def[token_type].name;
}

void json_get_token_str_repr(json_token_t* p_token, char* str, uint32_t str_len) {
	int consumed = snprintf(str, str_len, "%s", json_get_token_name(p_token->type));
	str += consumed;
	str_len -= consumed;

	switch (p_token->type) {
		case JSON_TOKEN_TYPE_VAL_NULL:
		case JSON_TOKEN_TYPE_START_OBJECT:
		case JSON_TOKEN_TYPE_END_OBJECT:
		case JSON_TOKEN_TYPE_VAL_START_ARRAY:
		case JSON_TOKEN_TYPE_VAL_END_ARRAY:
		case JSON_TOKEN_TYPE_MEMBER_DELIM:
		case JSON_TOKEN_TYPE_NAME_VAL_DELIM:
			break;
		case JSON_TOKEN_TYPE_VAL_STRING:
			snprintf(str, MIN(str_len, p_token->value.string.length), ": \"%s\"", p_token->value.string.data);
			break;
		case JSON_TOKEN_TYPE_VAL_BOOLEAN:
			if (p_token->value.boolean) {
				snprintf(str, str_len, ": true");
				break;
			}
			snprintf(str, str_len, ": false");
			break;
		case JSON_TOKEN_TYPE_VAL_NUMBER:
			snprintf(str, str_len, ": %f", p_token->value.number);
			break;
		case JSON_TOKEN_TYPE_UNDEFINED:
		case JSON_TOKEN_TYPE_WHITESPACE:
		case JSON_TOKEN_TYPE_COUNT:
			snprintf(str, str_len, ": <UNKNOWN>");
			break;
	}
}