//
// Created by tholz on 12.06.2022.
//

#include <string.h>
#include "test_json.h"
#include "json.h"

#define LOG_LEVEL    LOG_LEVEL_DEBUG
#include "testlib.h"

TEST_DEF(test_json_stringify, stringify_simple_key_value) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\"key\":\"value\"}";
	char *string = json_stringify(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_simple_key_value_pretty) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\n  \"key\": \"value\"\n}";
	char *string = json_stringify_pretty(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_multiple_values) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	key = "key2";
	value = (json_value_t) {.string = "value2"};
	ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	key = "key3";
	value = (json_value_t) {.string = "value3"};
	ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\"key\":\"value\",\"key2\":\"value2\",\"key3\":\"value3\"}";
	char *string = json_stringify(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_multiple_values_pretty) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	key = "key2";
	value = (json_value_t) {.string = "value2"};
	ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	key = "key3";
	value = (json_value_t) {.string = "value3"};
	ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\n  \"key\": \"value\",\n  \"key2\": \"value2\",\n  \"key3\": \"value3\"\n}";
	char *string = json_stringify_pretty(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_nested) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	json_object_t object2 = {0};
	char *key2 = "key2";
	json_value_t value2 = {.object = &object};
	ret = json_object_add_value(&object2, key2, value2, JSON_VALUE_TYPE_OBJECT);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\"key2\":{\"key\":\"value\"}}";
	char *string = json_stringify(&object2);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_nested_pretty) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	json_object_t object2 = {0};
	char *key2 = "key2";
	json_value_t value2 = {.object = &object};
	ret = json_object_add_value(&object2, key2, value2, JSON_VALUE_TYPE_OBJECT);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	char *expected_string = "{\n  \"key2\": {\n    \"key\": \"value\"\n  }\n}";
	char *string = json_stringify_pretty(&object2);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_array) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value1 = {.string = "value"};
	json_value_t value2 = {.string = "value2"};
	json_array_member_t array_members[2] = {
			{.value = value1, .type = JSON_VALUE_TYPE_STRING},
			{.value = value2, .type = JSON_VALUE_TYPE_STRING},
	};
	json_array_t array = {.length = 2};
	array.values[0] = &array_members[0];
	array.values[1] = &array_members[1];
	json_value_t value = {.array = &array};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_ARRAY);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *array_value = json_object_get_value(&object, key);
	TEST_ASSERT_NOT_NULL(array_value);
	json_value_t *array_value1 = json_value_get_array_member(array_value, 0);
	TEST_ASSERT_NOT_NULL(array_value1);
	json_value_t *array_value2 = json_value_get_array_member(array_value, 1);
	TEST_ASSERT_NOT_NULL(array_value2);

	char *expected_string = "{\"key\":[\"value\",\"value2\"]}";
	char *string = json_stringify(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_stringify, stringify_array_pretty) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value1 = {.string = "value"};
	json_value_t value2 = {.string = "value2"};
	json_array_member_t array_members[2] = {
			{.value = value1, .type = JSON_VALUE_TYPE_STRING},
			{.value = value2, .type = JSON_VALUE_TYPE_STRING},
	};
	json_array_t array = {.length = 2};
	array.values[0] = &array_members[0];
	array.values[1] = &array_members[1];
	json_value_t value = {.array = &array};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_ARRAY);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *array_value = json_object_get_value(&object, key);
	TEST_ASSERT_NOT_NULL(array_value);
	json_value_t *array_value1 = json_value_get_array_member(array_value, 0);
	TEST_ASSERT_NOT_NULL(array_value1);
	json_value_t *array_value2 = json_value_get_array_member(array_value, 1);
	TEST_ASSERT_NOT_NULL(array_value2);

	char *expected_string = "{\n  \"key\": [\n    \"value\",\n    \"value2\"\n  ]\n}";
	char *string = json_stringify_pretty(&object);
	log_debug("%s\n", string);
	TEST_EXPECT_EQ_STRING(string, expected_string, strlen(expected_string));

	free(string);

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_stringify() {
	TEST_GROUP_REG(test_json_stringify);
	TEST_REG(test_json_stringify, stringify_simple_key_value);
	TEST_REG(test_json_stringify, stringify_simple_key_value_pretty);
	TEST_REG(test_json_stringify, stringify_multiple_values);
	TEST_REG(test_json_stringify, stringify_multiple_values_pretty);
	TEST_REG(test_json_stringify, stringify_nested);
	TEST_REG(test_json_stringify, stringify_nested_pretty);
	TEST_REG(test_json_stringify, stringify_array);
	TEST_REG(test_json_stringify, stringify_array_pretty);
	TESTS_RUN();
}
