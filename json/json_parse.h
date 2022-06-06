//
// Created by tholz on 06.06.2022.
//

#ifndef JSON_PARSER_JSON_PARSE_H
#define JSON_PARSER_JSON_PARSE_H

#include "json.h"
#include "json_lex.h"

json_ret_code_t json_parse_object(json_token_t* tokens, uint32_t num_tokens, json_object_t* p_object);

#endif //JSON_PARSER_JSON_PARSE_H
