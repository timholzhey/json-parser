//
// Created by tholz on 28.11.2021.
//

#ifndef TESTLIB_TEST_H
#define TESTLIB_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_TEST_COUNT 100
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define TEST_ASSERT(assertion) \
    if (!(assertion)) { \
        log_error("\tAssertion failed: Expected 1, got 0 at %s:%d", __FILE__, __LINE__); \
        test_failed();          \
        return 1; \
    }
#define TEST_EXPECT(assertion) \
    if (!(assertion)) { \
        log_error("\tAssertion failed: Expected 1, got 0 at %s:%d", __FILE__, __LINE__); \
        test_failed();          \
    }

#define TEST_ASSERT_TRUE(assertion) TEST_ASSERT(assertion)
#define TEST_EXPECT_TRUE(assertion) TEST_EXPECT(assertion)

#define TEST_ASSERT_FALSE(assertion) TEST_ASSERT(!(assertion))
#define TEST_EXPECT_FALSE(assertion) TEST_EXPECT(!(assertion))

#define TEST_ASSERT_EQ(a, b) TEST_ASSERT((a) == (b))
#define TEST_EXPECT_EQ(a, b) TEST_EXPECT((a) == (b))

#define TEST_DEF(test_group_name, test_name) \
    int test_group_name##_##test_name()

#define TEST_REG(test_group_name, test_name) \
    register_test(#test_group_name, #test_name, test_group_name##_##test_name)

#define TESTS_RUN() \
    return run_tests()

#define STRINGIFY(x) #x

// read file using fopen, log_error and return 1 if failed to open or failed to allocate, create _size variable
#define TEST_READ_FILE(file_buffer, file_path) \
	char* file_buffer = NULL; \
	size_t file_buffer##_size = 0; \
	FILE* file_buffer##_file = fopen(file_path, "r"); \
	if (!file_buffer##_file) { \
		log_error("\tFailed to open file: %s", file_path); \
		return 1; \
	} \
	fseek(file_buffer##_file, 0, SEEK_END); \
	file_buffer##_size = ftell(file_buffer##_file); \
	rewind(file_buffer##_file); \
	file_buffer = malloc(file_buffer##_size + 1); \
	if (!file_buffer) { \
		log_error("\tFailed to allocate memory for file: %s", file_path); \
		return 1; \
	} \
    g_current_test.allocated_memory[g_current_test.allocated_memory_count++] = file_buffer; \
	if (fread(file_buffer, file_buffer##_size, 1, file_buffer##_file) != 1) { \
		log_error("\tFailed to read file: %s", file_path); \
		return 1; \
	} \
	file_buffer[file_buffer##_size] = '\0'; \
	fclose(file_buffer##_file);

#define TEST_PRINT_BUFFER(buffer_name) \
	for (size_t i = 0; i < buffer_name##_size; i++) { \
		if (isprint(buffer_name[i]) || isspace(buffer_name[i])) { \
			printf("%c", buffer_name[i]); \
		} else { \
			printf("[%02X]", buffer_name[i]); \
		} \
	} \
	printf("\n");

#define TEST_CLEAN_UP() \
	for (size_t i = 0; i < g_current_test.allocated_memory_count; i++) { \
		free(g_current_test.allocated_memory[i]); \
	} \

#define TEST_CLEAN_UP_AND_RETURN(return_value) \
	TEST_CLEAN_UP(); \
	return return_value;

#define log_error(message, ...) \
    printf("\033[0;31m" message "\033[0m\n", ##__VA_ARGS__)
#define log_success(message, ...) \
    printf("\033[0;32m" message "\033[0m\n", ##__VA_ARGS__)
#define log_warning(message, ...) \
    printf("\033[0;33m" message "\033[0m\n", ##__VA_ARGS__)
#define log_highlight(message, ...) \
    printf("\033[0;34m" message "\033[0m\n", ##__VA_ARGS__)
#define log_info(message, ...) \
    printf("\033[0;37m" message "\033[0m\n", ##__VA_ARGS__)

typedef struct {
    const char* test_group_name;
    const char* test_name;
    int (*test_function)(void);
    int test_passed;
	char* allocated_memory[10];
	uint8_t allocated_memory_count;
} test_t;

typedef struct {
    test_t tests[MAX_TEST_COUNT];
    int test_count;
    int test_passed;
    int test_failed;
} test_registry_t;

extern test_t g_current_test;

void register_test(const char* test_group_name, const char* test_name, void* test_function);
int run_tests();
void test_failed();

#endif //TESTLIB_TEST_H
