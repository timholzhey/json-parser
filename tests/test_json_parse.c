//
// Created by tholz on 06.06.2022.
//

#include <string.h>
#include "test_json.h"
#include "json/json_parse.h"

#define LOG_LEVEL    LOG_LEVEL_DEBUG
#include "testlib.h"

TEST_DEF(test_json_parse, parse_complete) {
	TEST_READ_FILE(buffer, "tests/files/complete.json");
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, strlen(buffer), &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "glossary");
	TEST_ASSERT_NOT_NULL(val);
	TEST_ASSERT_EQ_U8(json_object_get_value_type(&object, "glossary"), JSON_VALUE_TYPE_OBJECT);
	json_value_t *val2 = json_object_get_value(val->object, "title");
	TEST_ASSERT_NOT_NULL(val2);
	TEST_EXPECT_EQ_STRING(val2->string, "example glossary", strlen("example glossary"));
	json_value_t *val3 = json_object_get_value(val->object, "GlossDiv");
	TEST_ASSERT_NOT_NULL(val3);
	json_value_t *val4 = json_object_get_value(val3->object, "title");
	TEST_ASSERT_NOT_NULL(val4);
	TEST_EXPECT_EQ_STRING(val4->string, "S", strlen("S"));
	json_value_t *val5 = json_object_get_value(val3->object, "GlossList");
	TEST_ASSERT_NOT_NULL(val5);
	json_value_t *val6 = json_object_get_value(val5->object, "GlossEntry");
	TEST_ASSERT_NOT_NULL(val6);
	
	// test variables
	json_value_t *val7 = json_object_get_value(&object, "testTrue");
	TEST_ASSERT_NOT_NULL(val7);
	TEST_EXPECT_TRUE(val7->boolean);
	json_value_t *val8 = json_object_get_value(&object, "testFalse");
	TEST_ASSERT_NOT_NULL(val8);
	TEST_EXPECT_FALSE(val8->boolean);
	json_value_t *val9 = json_object_get_value(&object, "testNull");
	TEST_ASSERT_NOT_NULL(val9);
	json_value_t *val10 = json_object_get_value(&object, "testNumInt");
	TEST_ASSERT_NOT_NULL(val10);
	TEST_EXPECT_EQ_DOUBLE(val10->number, 1);
	json_value_t *val11 = json_object_get_value(&object, "testNumFloat");
	TEST_ASSERT_NOT_NULL(val11);
	TEST_EXPECT_EQ_DOUBLE(val11->number, 1.1);
	json_value_t *val12 = json_object_get_value(&object, "testNumExponent");
	TEST_ASSERT_NOT_NULL(val12);
	TEST_EXPECT_EQ_DOUBLE(val12->number, 1e1);
	json_value_t *val13 = json_object_get_value(&object, "testNumNegativeInt");
	TEST_ASSERT_NOT_NULL(val13);
	TEST_EXPECT_EQ_DOUBLE(val13->number, -1);
	json_value_t *val14 = json_object_get_value(&object, "testNumArray");
	TEST_ASSERT_NOT_NULL(val14);
	TEST_EXPECT_EQ_U32(val14->array->length, 3);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val14, 0)->number, 1);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val14, 1)->number, 2);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val14, 2)->number, 3);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_simple_key_value) {
	const char *buffer = "{\"key\":\"value\"}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "key");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_TRUE(json_object_has_key(&object, "key"));
	TEST_EXPECT_EQ_STRING(val->string, "value", strlen("value"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_number_array) {
	const char *buffer = "{\"key\": [1,2,3]}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "key");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_EQ_U32(val->array->length, 3);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 0)->number, 1);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 1)->number, 2);
	TEST_EXPECT_EQ_DOUBLE(json_value_get_array_member(val, 2)->number, 3);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_nested) {
	const char *buffer = "{\"key\": {\"key2\": \"value2\"}}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "key");
	TEST_ASSERT_NOT_NULL(val);
	json_value_t *val2 = json_object_get_value(val->object, "key2");
	TEST_ASSERT_NOT_NULL(val2);
	TEST_EXPECT_EQ_STRING(val2->string, "value2", strlen("value2"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_double_nested) {
	const char *buffer = "{\"key\": {\"key2\": {\"key3\": \"value3\"}}}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "key");
	TEST_ASSERT_NOT_NULL(val);
	json_value_t *val2 = json_object_get_value(val->object, "key2");
	TEST_ASSERT_NOT_NULL(val2);
	json_value_t *val3 = json_object_get_value(val2->object, "key3");
	TEST_ASSERT_NOT_NULL(val3);
	TEST_EXPECT_EQ_STRING(val3->string, "value3", strlen("value3"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_garbage_after_end) {
	const char *buffer = "{\"key\":\"value\"}\"garbage\"";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_FAIL);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_multiple_keys) {
	const char *buffer = "{\"key\":\"value\", \"key2\":\"value2\", \"key3\":\"value3\"}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t object;
	json_ret_code_t ret = json_parse(buffer, buffer_size, &object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	json_value_t *val = json_object_get_value(&object, "key");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_EQ_STRING(val->string, "value", strlen("value"));
	val = json_object_get_value(&object, "key2");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_EQ_STRING(val->string, "value2", strlen("value2"));
	val = json_object_get_value(&object, "key3");
	TEST_ASSERT_NOT_NULL(val);
	TEST_EXPECT_EQ_STRING(val->string, "value3", strlen("value3"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_parse() {
	TEST_GROUP_REG(test_json_parse);
	TEST_REG(test_json_parse, parse_complete);
	TEST_REG(test_json_parse, parse_simple_key_value);
	TEST_REG(test_json_parse, parse_number_array);
	TEST_REG(test_json_parse, parse_nested);
	TEST_REG(test_json_parse, parse_double_nested);
	TEST_REG(test_json_parse, parse_garbage_after_end);
	TEST_REG(test_json_parse, parse_multiple_keys);
	TESTS_RUN();
}
