#include "foo.h"

#include <stdio.h>
#include <stdlib.h>

FooMessage *foo_new(char *name) {
    FooMessage *msg = malloc(sizeof(FooMessage));
    msg->count = 0;
    msg->name = name;
    return msg;
}

void foo_print(FooMessage *msg) {
    printf("Hello, %s! (%d times)\n", msg->name, msg->count);
}
