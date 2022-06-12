//
// Created by tholz on 29.05.2022.
//

#include "test_json.h"

int main() {
	test_json_lex();
	test_json_parse();
	test_json_build();
	test_json_stringify();
    return 0;
}
