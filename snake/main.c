#include "foo.h"

#include <stdio.h>

int main(void) {
    char *program_name = "snake";
    Foo_Message *msg = foo_new(program_name);
    for (int i = 0; i < 10; i++) {
        foo_print(msg);
    }
}
