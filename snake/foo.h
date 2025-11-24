#ifndef FOO_H
#define FOO_H

typedef struct {
    int count;
    char *name;
} Foo_Message;

Foo_Message *foo_new(char *name);
void foo_print(Foo_Message *msg);

#endif
