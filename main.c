//
// Created by tholz on 29.05.2022.
//

#include "test_json.h"
#include "json.h"
#include <stdio.h>

#define RUN_TESTS	0

int main() {
#if defined RUN_TESTS && RUN_TESTS == 1
	test_json_lex();
	test_json_parse();
	test_json_build();
	test_json_stringify();
#else
	json_parse_string("{\"key\":\"value\"}", obj);

	printf("Parser returned: %d\n", obj_return);
	printf("Has key 'key': %d\n", json_object_has_key(&obj, "key"));
	printf("Value is: %s\n", json_object_get_value(&obj, "key")->string);
	printf("Value type: %u\n", json_object_get_value_type(&obj, "key"));
	printf("Minimal print: %s\n", json_stringify(&obj));
	printf("Pretty print: %s\n", json_stringify_pretty(&obj));
#endif
    return 0;
}
