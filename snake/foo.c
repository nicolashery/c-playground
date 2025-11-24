#include "foo.h"

#include <stdio.h>
#include <stdlib.h>

Foo_Message *foo_new(char *name) {
    Foo_Message *msg = malloc(sizeof(Foo_Message));
    msg->count = 0;
    msg->name = name;
    return msg;
}

void foo_print(Foo_Message *msg) {
    printf("Hello, %s! (%d times)\n", msg->name, msg->count);
}
