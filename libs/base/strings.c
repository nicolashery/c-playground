#include "strings.h"

#include "arena.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static String string_zero(void) {
    return (String){0};
}

static bool string_is_zero(String str) {
    return str.data == NULL;
}

String string_create(const char *cstr) {
    if (cstr == NULL) {
        return string_zero();
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
    assert((str.length == 0 || start < str.length) && "start out of bounds");
    assert(end <= str.length && "end out of bounds");

    return (String){
        .data = str.data + start,
        .length = end - start,
    };
}

bool string_equals(String a, String b) {
    if (string_is_zero(a) || string_is_zero(b)) {
        return string_is_zero(a) && string_is_zero(b);
    }

    if (a.length != b.length) {
        return false;
    }

    return memcmp(a.data, b.data, a.length) == 0;
}

char *string_to_cstr(Arena *arena, String str) {
    if (string_is_zero(str)) {
        return NULL;
    }

    char *result = arena_push_array(arena, char, str.length + 1);
    memcpy(result, str.data, str.length);
    result[str.length] = '\0';
    return result;
}
