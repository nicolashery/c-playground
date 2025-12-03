// NOLINTBEGIN(bugprone-suspicious-include)
#include "stats.c"
// #include "foo.c"
// NOLINTEND(bugprone-suspicious-include)

int main(int argc, char *argv[]) {
    return stats_main(argc, argv);
    // return foo_main(argc, argv);
}
