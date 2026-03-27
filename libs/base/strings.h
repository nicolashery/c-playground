#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

typedef struct {
    const char *data;
    size_t length;
} String;

String string_create(const char *cstr);

String string_create_len(const char *data, size_t len);

#endif
