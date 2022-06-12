//
// Created by tholz on 11.06.2022.
//

#include <string.h>
#include "test_json.h"
#include "json.h"

#define LOG_LEVEL    LOG_LEVEL_DEBUG
#include "testlib.h"

TEST_DEF(test_json_build, build_simple_key_value) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	TEST_EXPECT_TRUE(json_object_has_key(&object, key));
	TEST_EXPECT_EQ_STRING(json_object_get_value(&object, key)->string, "value", strlen("value"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_build, build_nested) {
	json_object_t object = {0};
	char *key = "key";
	json_value_t value = {.string = "value"};
	json_ret_code_t ret = json_object_add_value(&object, key, value, JSON_VALUE_TYPE_STRING);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	TEST_EXPECT_TRUE(json_object_has_key(&object, key));
	TEST_EXPECT_EQ_STRING(json_object_get_value(&object, key)->string, "value", strlen("value"));

	json_object_t object2 = {0};
	char *key2 = "key2";
	json_value_t value2 = {.object = &object};
	ret = json_object_add_value(&object2, key2, value2, JSON_VALUE_TYPE_OBJECT);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	TEST_EXPECT_TRUE(json_object_has_key(&object2, key2));
	TEST_EXPECT_EQ_STRING(json_object_get_value(json_object_get_value(&object2, key2)->object, key)->string, "value",
						  strlen("value"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_build, build_array) {
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
	TEST_EXPECT_TRUE(json_object_has_key(&object, key));
	json_value_t *array_value = json_object_get_value(&object, key);
	TEST_ASSERT_NOT_NULL(array_value);
	json_value_t *array_value1 = json_value_get_array_member(array_value, 0);
	TEST_ASSERT_NOT_NULL(array_value1);
	TEST_EXPECT_EQ_STRING(array_value1->string, "value", strlen("value"));
	json_value_t *array_value2 = json_value_get_array_member(array_value, 1);
	TEST_ASSERT_NOT_NULL(array_value2);
	TEST_EXPECT_EQ_STRING(array_value2->string, "value2", strlen("value2"));

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_build() {
	TEST_GROUP_REG(test_json_build);
	TEST_REG(test_json_build, build_simple_key_value);
	TEST_REG(test_json_build, build_nested);
	TEST_REG(test_json_build, build_array);
	TESTS_RUN();
}
