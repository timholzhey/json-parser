# JSON Parser

Minimal JSON parser in C.

## Headers

```c
#include "json.h"
```

## Library functions

```c
json_parse_string(string, object_name);
json_parse(p_buffer, size, p_object);

json_object_get_value(p_object, key);
json_object_get_value_type(p_object, key);
json_object_has_key(p_object, key);

json_value_get_array_member(p_value, index);

json_object_add_value(p_object, key, value, type);

json_object_free(p_object);

json_stringify(p_object);
json_stringify_pretty(p_object);
```

## Sample application

```c
json_parse_string("{\"key\":\"value\"}", obj);

printf("Parser returned: %d\n", obj_return);
printf("Has key 'key': %d\n", json_object_has_key(&obj, "key"));
printf("Value is: %s\n", json_object_get_value(&obj, "key")->string);
printf("Value type: %u\n", json_object_get_value_type(&obj, "key"));
printf("Minimal print: %s\n", json_stringify(&obj));
printf("Pretty print: %s\n", json_stringify_pretty(&obj));
```

## Tests

```yaml
tests/test_json_lex.c
tests/test_json_parse.c
tests/test_json_build.c
tests/test_json_stringify.c
```