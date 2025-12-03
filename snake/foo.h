#ifndef FOO_H
#define FOO_H

typedef struct {
    int count;
    char *name;
} FooMessage;

FooMessage *foo_new(char *name);
void foo_print(FooMessage *msg);

#endif
