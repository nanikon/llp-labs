#include "test/unit_test/unit_test.h"
#include "test/stress_test/stress_test.h"
#include "api/api.h"
#include <cstring>

extern unit_test_func unit_tests[UNIT_TEST_COUNT];
extern stress_test_func stress_tests[STRESS_TEST_COUNT];

const char* argument_description = "u if you want to run unit test, s if you want to run stress test";

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments: %d, expect 1: %s\n", argc - 1, argument_description);
        return -1;
    }
    if (strcmp(argv[1], "u") == 0) {
        printf("<============ Unit tests started ============>\n");
        for (int i = 0; i < UNIT_TEST_COUNT; i++) {
            unit_tests[i]();
        };
        printf("<============ Unit tests finihshed ============>\n");
    } else if (strcmp(argv[1], "s") == 0) {
        printf("<============ Stress tests started ============>\n");
        size_t count = 20000;
        for (int i = 0; i < STRESS_TEST_COUNT; i++) {
            stress_tests[i](count);
        }
        printf("<============ Stress tests finihshed ============>\n");
    } else {
        fprintf(stderr, "Invalid argument %s, expect: %s\n", argv[1], argument_description);
        return -1;
    }
}
