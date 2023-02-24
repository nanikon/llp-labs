#include "file.h"

size_t write_buffer_to_file(int32_t fd, size_t offset, const void* buffer, size_t elem_size, size_t count) {
    size_t result_offset = lseek(fd, offset, SEEK_SET); 
    if (result_offset != offset) {
        printf("Cannot seek in file. Errno: %d\n ", errno);
        return -1;
    }

    size_t write_len = elem_size * count;
    size_t result_len = write(fd, buffer, write_len); 
    if (result_len < write_len) {
        printf("Cannot write to file. Errno: %d\n ", errno);
        return -1;
    }

    return offset + write_len;
}

size_t read_buffer_from_file(int32_t fd, size_t offset, void* buffer, size_t elem_size, size_t count) {
    size_t offset_result = lseek(fd, offset, SEEK_SET); 
    if (offset_result != offset) {
        printf("Cannot seek in file. Errno: %d\n ", errno);
        return -1;
    }

    size_t read_len = elem_size * count;
    size_t result_len = read(fd, buffer, read_len);
    if (result_len < read_len) {
        printf("Cannot read from file. Errno: %d\n ", errno);
        return -1;
    }

    return offset + read_len;
}

int32_t open_file(const char* filename) {
    int32_t fd = open(filename, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);

    if (fd == -1) {
        printf("Cannot open file. Errno: %d\n ", errno);
    }

    return fd;
}

void close_file(int32_t fd) {
    close(fd);
}

size_t get_file_len(const int32_t fd) {
    struct stat64 file_stat = {0}; 

    if (fstat64(fd, &file_stat) != 0) {
        printf("Cannot get file stat. Errno: %d\n ", errno);
    }

    return file_stat.st_size;
}
