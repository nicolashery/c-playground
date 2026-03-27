#include "strings.h"

#include <string.h>

String string_create(const char *cstr) {
    if (cstr == NULL) {
        return (String){0};
    }

    return (String){
        .data = cstr,
        .length = strlen(cstr),
    };
}

String string_create_len(const char *data, size_t len) {
    return (String){
        .data = data,
        .length = len,
    };
}
