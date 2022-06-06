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

	json_object_t* p_object = NULL;
	json_ret_code_t ret = json_parse(buffer, strlen(buffer), p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	TEST_EXPECT_NOT_NULL(p_object);

	TEST_CLEAN_UP_AND_RETURN(0);
}

TEST_DEF(test_json_parse, parse_simple) {
	const char* buffer = "{\"key\":\"value\"}";
	size_t buffer_size = strlen(buffer);
	TEST_PRINT_BUFFER(buffer);

	json_object_t* p_object = NULL;
	json_ret_code_t ret = json_parse(buffer, buffer_size, p_object);
	TEST_EXPECT_EQ_U8(ret, JSON_RETVAL_OK);
	TEST_EXPECT_NOT_NULL(p_object);

	TEST_CLEAN_UP_AND_RETURN(0);
}

int test_json_parse() {
	TEST_GROUP_REG(test_json_parse);
	//TEST_REG(test_json_parse, parse_complete);
	TEST_REG(test_json_parse, parse_simple);
	TESTS_RUN();
}
