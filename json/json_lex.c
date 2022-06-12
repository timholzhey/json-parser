//
// Created by tholz on 29.05.2022.
//

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "json_lex.h"

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

#define JSON_LEX_CHAR_BUFFER_SIZE	1000 * 1024

typedef enum {
	JSON_LEX_ERRCODE_OK,
	JSON_LEX_ERRCODE_UNEXPECTED_EOF,
	JSON_LEX_ERRCODE_UNEXPECTED_TOKEN,
	JSON_LEX_ERRCODE_EXPECTED_DIGIT,
	JSON_LEX_ERRCODE_NAN,
	JSON_LEX_ERRCODE_ILLEGAL_ESCAPE_SEQUENCE,
	JSON_LEX_ERRCODE_INVALID_UNICODE_CHAR,
} json_lex_errcode;

static struct {
	char buffer[JSON_LEX_CHAR_BUFFER_SIZE];
	uint16_t buffer_len;
	json_token_t token;
	json_ret_code_t ret_code;
	uint16_t line;
	uint16_t column;
	uint16_t line_start;
	json_lex_errcode err_code;
} m_json_lex;

json_ret_code_t json_strcmp_partial(const char* expect_str, const char* actual_str,
												  uint16_t expect_str_len, uint16_t actual_str_len) {
	if (expect_str_len < actual_str_len || actual_str_len == 0) {
		return JSON_RETVAL_FAIL;
	}
	if (expect_str_len == actual_str_len) {
		return strcmp(expect_str, actual_str) == 0 ? JSON_RETVAL_OK : JSON_RETVAL_FAIL;
	}
	return strncmp(expect_str, actual_str, actual_str_len) == 0 ? JSON_RETVAL_INCOMPLETE : JSON_RETVAL_FAIL;
}

json_ret_code_t json_str_unescape(char* str_dest, const char* str_src, uint16_t str_len) {
	uint16_t j = 0;
	for (uint16_t i = 0; i < str_len; i++) {
		if (str_src[i] == '\\') {
			if (i + 1 >= str_len) {
				return JSON_RETVAL_INCOMPLETE;
			}
			switch (str_src[++i]) {
				case '"': str_dest[j++] = '"'; break;
				case '\\': str_dest[j++] = '\\'; break;
				case '/': str_dest[j++] = '/'; break;
				case 'b': str_dest[j++] = '\b'; break;
				case 'f': str_dest[j++] = '\f'; break;
				case 'n': str_dest[j++] = '\n'; break;
				case 'r': str_dest[j++] = '\r'; break;
				case 't': str_dest[j++] = '\t'; break;
				case 'u':
					i++;
					if (i + 4 >= str_len) {
						return JSON_RETVAL_INCOMPLETE;
					}
					uint16_t unicode_char = strtol(&str_src[i], NULL, 16);
					str_dest[j++] = (char) unicode_char;
					i += 4;
					break;
				default:
					return JSON_RETVAL_ILLEGAL;
			}
			continue;
		}
		str_dest[j++] = str_src[i];
	}
	return JSON_RETVAL_OK;
}

typedef enum {
	JSON_PARSE_NUMBER_STATE_INIT,
	JSON_PARSE_NUMBER_STATE_SIGN,
	JSON_PARSE_NUMBER_STATE_SWITCH1,
	JSON_PARSE_NUMBER_STATE_ZERO,
	JSON_PARSE_NUMBER_STATE_DIGIT_NON_ZERO,
	JSON_PARSE_NUMBER_STATE_DIGIT,
	JSON_PARSE_NUMBER_STATE_SWITCH2,
	JSON_PARSE_NUMBER_STATE_FRAC_DOT,
	JSON_PARSE_NUMBER_STATE_FRAC_DIGIT,
	JSON_PARSE_NUMBER_STATE_EXP_E,
	JSON_PARSE_NUMBER_STATE_EXP_SIGN,
	JSON_PARSE_NUMBER_STATE_EXP_DIGIT,
	JSON_PARSE_NUMBER_STATE_FINISH,
} json_parse_number_state_t;

#define JSON_PARSE_NUMBER_NEXT(_state)			state = _state; i++; continue;
#define JSON_PARSE_NUMBER_GOTO(_state)			state = _state; continue;
#define JSON_PARSE_NUMBER_NEXT_ON(c, _state)	if ((c) == str_src[i]) { JSON_PARSE_NUMBER_NEXT(_state); }

static inline double pow10fast(uint32_t n) {
	if (n <= 10) {
		uint32_t pow10_table[] = {
			1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
		};
		return (double) pow10_table[n];
	}
	uint32_t i;
	double result = 1.0f;
	for (i = 0; i < n; i++) {
		result *= 10.0f;
	}
	return result;
}

json_ret_code_t json_parse_number(double *p_dest, const char* str_src, uint16_t str_len) {
	json_parse_number_state_t state = JSON_PARSE_NUMBER_STATE_INIT;
	bool sign = false, exp_sign = false;
	uint32_t integer = 0, exp = 0, i = 0;
	double frac = 0.0f, frac_part = 10.0f;

	while (i < str_len) {
		switch (state) {
			case JSON_PARSE_NUMBER_STATE_INIT:
				JSON_PARSE_NUMBER_NEXT_ON('-', JSON_PARSE_NUMBER_STATE_SIGN);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_SWITCH1);
			case JSON_PARSE_NUMBER_STATE_SIGN:
				sign = true;
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_SWITCH1);
			case JSON_PARSE_NUMBER_STATE_SWITCH1:
				JSON_PARSE_NUMBER_NEXT_ON('0', JSON_PARSE_NUMBER_STATE_ZERO);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_DIGIT_NON_ZERO);
			case JSON_PARSE_NUMBER_STATE_ZERO:
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_SWITCH2);
			case JSON_PARSE_NUMBER_STATE_DIGIT_NON_ZERO:
				if (str_src[i] > '0' && str_src[i] <= '9') {
					integer = str_src[i] - '0';
					state = JSON_PARSE_NUMBER_STATE_DIGIT;
					i++;
					break;
				}
				JSON_PARSE_NUMBER_NEXT_ON('.', JSON_PARSE_NUMBER_STATE_FRAC_DOT);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_SWITCH2);
			case JSON_PARSE_NUMBER_STATE_DIGIT:
				if (str_src[i] >= '0' && str_src[i] <= '9') {
					integer = integer * 10 + str_src[i] - '0';
					i++;
					break;
				}
				JSON_PARSE_NUMBER_NEXT_ON('.', JSON_PARSE_NUMBER_STATE_FRAC_DOT);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_SWITCH2);
			case JSON_PARSE_NUMBER_STATE_SWITCH2:
				JSON_PARSE_NUMBER_NEXT_ON('.', JSON_PARSE_NUMBER_STATE_FRAC_DOT);
				JSON_PARSE_NUMBER_NEXT_ON('e', JSON_PARSE_NUMBER_STATE_EXP_E);
				JSON_PARSE_NUMBER_NEXT_ON('E', JSON_PARSE_NUMBER_STATE_EXP_E);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_FINISH);
			case JSON_PARSE_NUMBER_STATE_FRAC_DOT:
				if (str_src[i] >= '0' && str_src[i] <= '9') {
					frac += (double) (str_src[i] - '0') / frac_part;
					frac_part *= 10.0f;
					state = JSON_PARSE_NUMBER_STATE_FRAC_DIGIT;
					i++;
					break;
				}
				m_json_lex.err_code = JSON_LEX_ERRCODE_EXPECTED_DIGIT;
				return JSON_RETVAL_ILLEGAL;
			case JSON_PARSE_NUMBER_STATE_FRAC_DIGIT:
				if (str_src[i] >= '0' && str_src[i] <= '9') {
					frac += (double) (str_src[i] - '0') / frac_part;
					frac_part *= 10.0f;
					i++;
					break;
				}
				JSON_PARSE_NUMBER_NEXT_ON('e', JSON_PARSE_NUMBER_STATE_EXP_E);
				JSON_PARSE_NUMBER_NEXT_ON('E', JSON_PARSE_NUMBER_STATE_EXP_E);
				JSON_PARSE_NUMBER_GOTO(JSON_PARSE_NUMBER_STATE_FINISH);
			case JSON_PARSE_NUMBER_STATE_EXP_E:
				JSON_PARSE_NUMBER_NEXT_ON('+', JSON_PARSE_NUMBER_STATE_EXP_SIGN);
				if (str_src[i] == '-') {
					exp_sign = true;
					state = JSON_PARSE_NUMBER_STATE_EXP_SIGN;
					i++;
					break;
				}
				state = JSON_PARSE_NUMBER_STATE_EXP_DIGIT;
				break;
			case JSON_PARSE_NUMBER_STATE_EXP_SIGN:
				if (str_src[i] >= '0' && str_src[i] <= '9') {
					state = JSON_PARSE_NUMBER_STATE_EXP_DIGIT;
					i++;
					break;
				}
				m_json_lex.err_code = JSON_LEX_ERRCODE_EXPECTED_DIGIT;
				return JSON_RETVAL_ILLEGAL;
			case JSON_PARSE_NUMBER_STATE_EXP_DIGIT:
				if (str_src[i] >= '0' && str_src[i] <= '9') {
					exp = exp * 10 + str_src[i] - '0';
					i++;
					break;
				}
				m_json_lex.err_code = JSON_LEX_ERRCODE_EXPECTED_DIGIT;
				return JSON_RETVAL_ILLEGAL;
			case JSON_PARSE_NUMBER_STATE_FINISH: {
				return JSON_RETVAL_FAIL;
			}
			default:
				return JSON_RETVAL_FAIL;
		}
	}

	// Valid finish states
	if (state == JSON_PARSE_NUMBER_STATE_FINISH || state == JSON_PARSE_NUMBER_STATE_DIGIT ||
		state == JSON_PARSE_NUMBER_STATE_DIGIT_NON_ZERO || state == JSON_PARSE_NUMBER_STATE_FRAC_DIGIT ||
		state == JSON_PARSE_NUMBER_STATE_EXP_DIGIT) {
		double integer_signed = integer * (sign ? -1.0 : 1.0f);
		double frac_signed = frac * (sign ? -1.0f : 1.0f);
		double exp_signed = exp_sign ? 1.0f / pow10fast(exp) : pow10fast(exp);
		double result =  (integer_signed + frac_signed) * exp_signed;
		*p_dest = result;
		return JSON_RETVAL_OK;
	}

	m_json_lex.err_code = JSON_LEX_ERRCODE_NAN;
	return JSON_RETVAL_INCOMPLETE;
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

	bool do_escape = false;
	uint16_t num_unicode_chars = 0;
	for (uint16_t i = 1; i < m_json_lex.buffer_len; i++) {
		if (num_unicode_chars > 0) {
			if ((m_json_lex.buffer[i] >= '0' && m_json_lex.buffer[i] <= '9') ||
				(m_json_lex.buffer[i] >= 'a' && m_json_lex.buffer[i] <= 'f') ||
				(m_json_lex.buffer[i] >= 'A' && m_json_lex.buffer[i] <= 'F')) {
				num_unicode_chars--;
				continue;
			}
			m_json_lex.err_code = JSON_LEX_ERRCODE_INVALID_UNICODE_CHAR;
			JSON_RETURN_BOOL(false);
		}
		if (do_escape) {
			if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_QUOTE ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSLASH ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_SLASH ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSPACE ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_FORMFEED ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_LINEFEED ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_CARRIAGE_RET ||
				m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_HORIZ_TAB) {
				do_escape = false;
				continue;
			}
			if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_UNICODE) {
				do_escape = false;
				num_unicode_chars = 4;
				continue;
			}
			m_json_lex.err_code = JSON_LEX_ERRCODE_ILLEGAL_ESCAPE_SEQUENCE;
			JSON_RETURN_BOOL(false);
		}
		if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_BACKSLASH) {
			do_escape = true;
			continue;
		}
		if (m_json_lex.buffer[i] == *JSON_TOKEN_STR_REPR_VAL_STRING_ESC_QUOTE) {
			JSON_RETURN_BOOL(false);
		}
	}
	if (m_json_lex.buffer_len >= 2 && m_json_lex.buffer[m_json_lex.buffer_len] == *JSON_TOKEN_STR_REPR_VAL_STRING_QUOTES) {
		m_json_lex.token.value.string.data = calloc(m_json_lex.buffer_len, sizeof(char));
		json_str_unescape(m_json_lex.token.value.string.data, m_json_lex.buffer + 1, m_json_lex.buffer_len - 1);
		m_json_lex.token.value.string.length = m_json_lex.buffer_len - 1;
	} else {
		return JSON_RETVAL_INCOMPLETE;
	}
	JSON_RETURN_BOOL(match);
}

JSON_IS_TOKEN_TYPE_FN_IMPL(JSON_TOKEN_TYPE_VAL_NUMBER) {
	return json_parse_number(&m_json_lex.token.value.number, m_json_lex.buffer, m_json_lex.buffer_len + 1);
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

char* json_lex_get_err_str(json_lex_errcode err) {
	switch (err) {
		case JSON_LEX_ERRCODE_UNEXPECTED_TOKEN:
			return "Unexpected token";
		case JSON_LEX_ERRCODE_UNEXPECTED_EOF:
			return "Unexpected end of file";
		case JSON_LEX_ERRCODE_ILLEGAL_ESCAPE_SEQUENCE:
			return "Illegal escape sequence";
		case JSON_LEX_ERRCODE_EXPECTED_DIGIT:
			return "Expected digit after";
		case JSON_LEX_ERRCODE_NAN:
			return "Not a number";
		case JSON_LEX_ERRCODE_INVALID_UNICODE_CHAR:
			return "Invalid unicode escape sequence in string";
		default:
			return "Unknown error";
	}
}

static void json_lex_error_handler(const char* p_input, uint32_t input_len, json_ret_code_t ret) {
	if (ret == JSON_RETVAL_ILLEGAL) {
		printf("\033[31mSyntaxError: %s \"%.*s\" at %u:%u\033[0m\n", json_lex_get_err_str(m_json_lex.err_code), m_json_lex.buffer_len + 1, m_json_lex.buffer, m_json_lex.line + 1, m_json_lex.column + 1);
		printf("%5u |     ", m_json_lex.line + 1);
		for (uint16_t i = m_json_lex.line_start; i < input_len && p_input[i] != '\n' && p_input[i] != '\r'; i++) {
			printf("%c", p_input[i]);
		}
		printf("\n");
		printf("      |     ");
		for (uint16_t i = 0; i < m_json_lex.column; i++) {
			printf(" ");
		}
		printf("\033[31m^\033[0m\n");
	} else {
		printf("\033[31mInternal Error\033[0m\n");
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

		switch (ret) {
			case JSON_RETVAL_OK:
				p_tokens[(*p_num_tokens)++] = token;
				break;
			case JSON_RETVAL_BUSY:
				break;
			case JSON_RETVAL_FINISHED:
				return JSON_RETVAL_OK;
			default:
				json_lex_error_handler(p_input, input_len, ret);
				return ret;
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

		if (num_matches == 0 || *p_consumed == input_len) {
			m_json_lex.column += m_json_lex.buffer_len;
			if (m_json_lex.buffer[0] == '\n' || m_json_lex.buffer[0] == '\r') {
				m_json_lex.line++;
				m_json_lex.column = 0;
			}
			if (m_json_lex.ret_code != JSON_RETVAL_OK) {
				return JSON_RETVAL_ILLEGAL;
			}
			bool return_token = false;

			// Found token
			if (m_json_lex.token.type != JSON_TOKEN_TYPE_UNDEFINED) {
				if ((json_token_type_def[m_json_lex.token.type].flags & JSON_TOKEN_FLAG_IGNORED) == 0) {
					copy_token(p_token, &m_json_lex.token);
					return_token = true;
				}
			}

			// Reset
			clear_token(&m_json_lex.token);
			m_json_lex.buffer_len = 0;
			memset(m_json_lex.buffer, 0, sizeof(m_json_lex.buffer));
			m_json_lex.ret_code = JSON_RETVAL_FAIL;
			m_json_lex.token.line = m_json_lex.line;
			m_json_lex.token.column = m_json_lex.column;
			m_json_lex.err_code = JSON_LEX_ERRCODE_UNEXPECTED_TOKEN;

			// Reached end of input
			if (num_matches == 1 && *p_consumed == input_len) {
				return return_token ? JSON_RETVAL_OK : JSON_RETVAL_FINISHED;
			}

			// Push back
			(*p_consumed)--;
			return return_token ? JSON_RETVAL_OK : JSON_RETVAL_BUSY;
		} else {
			m_json_lex.buffer_len++;
		}
	}

	return JSON_RETVAL_FINISHED;
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