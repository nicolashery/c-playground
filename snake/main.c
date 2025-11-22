#include <stdio.h>

typedef struct {
    int count;
    char *name;
} Some_Struct;

int main(void) {
    char *program_name = "snake";
    printf("Hello, %s!", program_name);
}
