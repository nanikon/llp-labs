#ifndef LLP_FILE_H
#define LLP_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

size_t write_buffer_to_file(int32_t fd, size_t offset, const void* buffer, size_t elem_size, size_t count);

size_t read_buffer_from_file(int32_t fd, size_t offset, void* buffer, size_t elem_size, size_t count);

int32_t open_file(const char* filename);

void close_file(int32_t fd);

size_t get_file_len(const int32_t fd);

#endif
