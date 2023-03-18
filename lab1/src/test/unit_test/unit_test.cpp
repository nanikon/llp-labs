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

void test_str_io_module() {
    int32_t fd = open_file("test");
    assert(fd);

    const char* first_str = "Hello world!";
    size_t result = write_null_term_str_to_file(fd, 0, first_str);
    assert(result != -1);

    const char* result_str = NULL;
    result = read_null_term_str_from_file(fd, 0, &(result_str));
    assert(result != -1);
    assert(strcmp(first_str, result_str)== 0);

    close_file(fd);
    printf("test str io module finished successful\n");
}

void test_block_io_module() {
    int32_t fd = open_file("test");
    assert(fd);
    struct file_descriptor ptr = { .fd = fd, .header = NULL };

    struct block source = { .next = 20, .prev = 30, .size = 100 };
    write_block(&source, &ptr, 5);

    struct block* result = read_block(&ptr, 5);

    assert(memcmp(&source, result, sizeof(struct block)) == 0);

    free_block(result);
    close_file(fd);
    printf("test block io module finished successful\n");
}

unit_test_func unit_tests[UNIT_TEST_COUNT] = {
    test_io_module,
    test_str_io_module,
    test_block_io_module
};
