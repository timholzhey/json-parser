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

## Tests

```yaml
tests/test_json_lex.c
tests/test_json_parse.c
tests/test_json_build.c
tests/test_json_stringify.c
```