#include "strings.h"

#include <assert.h>
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

String string_slice(String str, size_t start, size_t end) {
    assert(start <= end && "start should come before end");
    assert(start < str.length && "start out of bounds");
    assert(end <= str.length && "end out of bounds");

    return (String){
        .data = str.data + start,
        .length = end - start,
    };
}
