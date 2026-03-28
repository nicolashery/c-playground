#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

typedef struct {
    const char *data;
    size_t length;
} String;

String string_create(const char *cstr);

String string_create_len(const char *data, size_t len);

// Return a view (slice) into a string between start (inclusive) and end (exclusive)
String string_slice(String str, size_t start, size_t end);

#endif
