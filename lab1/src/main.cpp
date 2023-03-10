#include "test/unit_test/unit_test.h"
#include <cstring>

extern unit_test_func unit_tests[UNIT_TEST_COUNT];

const char* argument_description = "u if you want to run unit test, s if you want to run stress test";

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments: %d, expect 1: %s\n", argc - 1, argument_description);
        return -1;
    }
    if (strcmp(argv[1], "u") == 0) {
        printf("<============ Unit tests started ============>\n");
        for (auto & unit_test : unit_tests) {
            unit_test();
        };
        printf("<============ Unit tests finihshed ============>\n");
    } else if (strcmp(argv[1], "s") == 0) {
        printf("<============ Stress tests started ============>\n");
        printf("<============ Stress tests finihshed ============>\n");
    } else {
        fprintf(stderr, "Invalid argument %s, expect: %s\n", argv[1], argument_description);
        return -1;
    }
    return 0;
}
