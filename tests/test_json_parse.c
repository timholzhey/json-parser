//
// Created by tholz on 06.06.2022.
//

#include <string.h>
#include "test_json.h"
#include "json/json_parse.h"

#define LOG_LEVEL	LOG_LEVEL_DEBUG
#include "testlib.h"

TEST_DEF(test_json_parse, parse_complete) {
	TEST_READ_FILE(buffer, "tests/files/complete.json");
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, strlen(buffer), &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_simple_key_value) {
	const char* buffer = "{\"key\":\"value\"}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t* val = json_object_get_value(&p_object, "key");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_TRUE(json_object_has_key(&p_object, "key"));
	TEST_EXPECT_EQ_STRING(val->string, "value", strlen("value"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_number_array) {
	const char* buffer = "{\"key\": [1,2,3]}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t* val = json_object_get_value(&p_object, "key");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_EQ_U32(val->array->length, 3);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 0)->number, 1);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 1)->number, 2);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 2)->number, 3);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_nested) {
	const char* buffer = "{\"key\": {\"key2\": \"value2\"}}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t* val = json_object_get_value(&p_object, "key");
	TEST_ASSERT_NOT_NULL(val);
	json_value_t* val2 = json_object_get_value(val->object, "key2");
	TEST_ASSERT_NOT_NULL(val2);
	TEST_EXPECT_EQ_STRING(val2->string, "value2", strlen("value2"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_double_nested) {
	const char* buffer = "{\"key\": {\"key2\": {\"key3\": \"value3\"}}}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t* val = json_object_get_value(&p_object, "key");
	TEST_ASSERT_NOT_NULL(val);
	json_value_t* val2 = json_object_get_value(val->object, "key2");
	TEST_ASSERT_NOT_NULL(val2);
	json_value_t* val3 = json_object_get_value(val2->object, "key3");
	TEST_ASSERT_NOT_NULL(val3);
	TEST_EXPECT_EQ_STRING(val3->string, "value3", strlen("value3"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_garbage_after_end) {
	const char* buffer = "{\"key\":\"value\"}\"garbage\"";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t p_object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_FAIL);

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_parse() {
	TEST_GROUP_REG(test_json_parse);
	//TEST_REG(test_json_parse, parse_complete);
	TEST_REG(test_json_parse, parse_simple_key_value);
	TEST_REG(test_json_parse, parse_number_array);
	TEST_REG(test_json_parse, parse_nested);
	TEST_REG(test_json_parse, parse_double_nested);
	TEST_REG(test_json_parse, parse_garbage_after_end);
	TESTS_RUN();
}
