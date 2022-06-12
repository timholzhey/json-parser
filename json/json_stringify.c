//
// Created by tholz on 12.06.2022.
//

#include "json_stringify.h"
#include <stdlib.h>
#include <stdio.h>

#define JSON_STRINGIFY_CHUNK_SIZE		1024
#define JSON_STRINGIFY_INDENT_SPACES	2

static struct {
	char *string;
	int max_string_length;
	int string_length;
} m_stringify;

#define JSON_STRINGIFY_REPORT_ERROR(msg, ...) { \
    printf("\033[31mFailed to stringify object: "); \
    printf(msg, ##__VA_ARGS__);                 \
    printf("\033[0m\n");                        \
}

static void string_append_object(const json_object_t *p_object, bool pretty, int level);

static inline void string_append(const char *cstr) {
	if (m_stringify.string_length + strlen(cstr) >= m_stringify.max_string_length) {
		m_stringify.max_string_length += JSON_STRINGIFY_CHUNK_SIZE;
		m_stringify.string = realloc(m_stringify.string, m_stringify.max_string_length);
	}
	memcpy(m_stringify.string + m_stringify.string_length, cstr, strlen(cstr));
	m_stringify.string_length += (int) strlen(cstr);
}

static inline void string_append_times(const char *cstr, int times) {
	for (int i = 0; i < times; i++) {
		string_append(cstr);
	}
}

static void string_append_member(json_value_t *value, json_value_type_t type, bool pretty, int level) {
	char buf[64];
	switch (type) {
		case JSON_VALUE_TYPE_STRING:
			string_append("\"");
			string_append(value->string);
			string_append("\"");
			break;
		case JSON_VALUE_TYPE_NUMBER:
			snprintf(buf, 64, "%f", value->number);
			string_append(buf);
			break;
		case JSON_VALUE_TYPE_BOOLEAN:
			string_append(value->boolean ? "true" : "false");
			break;
		case JSON_VALUE_TYPE_NULL:
			string_append("null");
			break;
		case JSON_VALUE_TYPE_ARRAY:
			string_append("[");
			if (pretty) {
				string_append("\n");
				string_append_times(" ", (level + 1) * JSON_STRINGIFY_INDENT_SPACES);
			}
			for (uint32_t j = 0; j < value->array->length; j++) {
				if (j > 0) {
					string_append(",");
					if (pretty) {
						string_append("\n");
						string_append_times(" ", (level + 1) * JSON_STRINGIFY_INDENT_SPACES);
					}
				}
				string_append_member(&value->array->values[j]->value, value->array->values[j]->type, pretty, level + 1);
			}
			if (pretty) {
				string_append("\n");
				string_append_times(" ", level * JSON_STRINGIFY_INDENT_SPACES);
			}
			string_append("]");
			break;
		case JSON_VALUE_TYPE_OBJECT:
			string_append_object(value->object, pretty, level);
			break;
		case JSON_VALUE_TYPE_UNDEFINED:
		default:
			JSON_STRINGIFY_REPORT_ERROR("Unknown value type");
			break;
	}
}

static void string_append_object(const json_object_t *p_object, bool pretty, int level) {
	string_append("{");
	if (pretty) {
		string_append("\n");
		string_append_times(" ", (level + 1) * JSON_STRINGIFY_INDENT_SPACES);
	}
	for (uint32_t i = 0; i < p_object->num_members; i++) {
		if (i > 0) {
			string_append(",");
			if (pretty) {
				string_append("\n");
				string_append_times(" ", (level + 1) * JSON_STRINGIFY_INDENT_SPACES);
			}
		}
		string_append("\"");
		string_append(p_object->members[i]->key);
		string_append("\":");
		if (pretty) string_append(" ");
		string_append_member(&p_object->members[i]->value, p_object->members[i]->type, pretty, level + 1);
	}
	if (pretty) {
		string_append("\n");
		string_append_times(" ", level * JSON_STRINGIFY_INDENT_SPACES);
	}
	string_append("}");
}

char *json_object_stringify(const json_object_t *p_object, bool pretty) {
	if (p_object == NULL) {
		JSON_STRINGIFY_REPORT_ERROR("Object is NULL");
		return NULL;
	}

	m_stringify.max_string_length = JSON_STRINGIFY_CHUNK_SIZE;
	m_stringify.string = malloc(m_stringify.max_string_length);
	m_stringify.string_length = 0;

	const json_object_t *p_current = p_object;
	string_append_object(p_current, pretty, 0);
	m_stringify.string[m_stringify.string_length] = '\0';

	return m_stringify.string;
}
