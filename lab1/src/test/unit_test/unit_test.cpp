#include "unit_test.h"

#define BUFFER_LEN 10

void test_io_module() {
    int32_t fd = open_file("test");
    assert(fd);

    int64_t buffer[BUFFER_LEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    size_t result = write_buffer_to_file(fd, 0, buffer, sizeof(int64_t), BUFFER_LEN);
    assert(result != -1);

    int64_t result_buffer[BUFFER_LEN];
    result = read_buffer_from_file(fd, 0, result_buffer, sizeof(int64_t), BUFFER_LEN);
    assert(result != -1);
    for (uint8_t i = 0; i < BUFFER_LEN; i++) {
        assert(buffer[i] == result_buffer[i]);
    }

    close_file(fd);
    printf("test io module finished successful\n");
};

unit_test_func unit_tests[UNIT_TEST_COUNT] = {
    test_io_module
};
