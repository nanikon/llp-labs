#include "file.h"

/**
 * @return filesize (offset + count of writed butes), or -1 if has a error
*/
size_t write_buffer_to_file(int32_t fd, size_t offset, const void* buffer, size_t elem_size, size_t count) {
    size_t result_offset = lseek(fd, offset, SEEK_SET); 
    if (result_offset != offset) {
        perror("Cannot seek in file");
        return -1;
    }

    size_t write_len = elem_size * count;
    size_t result_len = write(fd, buffer, write_len); 
    if (result_len < write_len) {
        perror("Cannot write to file");
        return -1;
    }

    return offset + write_len;
}

/**
 * Записывает нуль-терминированную строку в виде её длины и потом самой строки в файл
*/
size_t write_null_term_str_to_file(int32_t fd, size_t offset, const char* str) {
    size_t str_len = strlen(str);
    offset = write_buffer_to_file(fd, offset, &str_len, sizeof(size_t), 1);
    offset = write_buffer_to_file(fd, offset, str, sizeof(char), str_len);
    return offset;
}

size_t read_buffer_from_file(int32_t fd, size_t offset, void* buffer, size_t elem_size, size_t count) {
    size_t offset_result = lseek(fd, offset, SEEK_SET); 
    if (offset_result != offset) {
        perror("Cannot seek in file");
        return -1;
    }

    size_t read_len = elem_size * count;
    size_t result_len = read(fd, buffer, read_len);
    if (result_len < read_len) {
        perror("Cannot read from file");
        return -1;
    }

    return offset + read_len;
}

/**
 * Читает строку из файла, записанную как её длина + она сама из файла (сделано, чтобы не читать посимвольно до нуль-терминатора). 
 * В buffer возвращается ссылка на ссылку на строку
*/
size_t read_null_term_str_from_file(int32_t fd, size_t offset, const char** buffer) {
    size_t len = 0;
    offset = read_buffer_from_file(fd, offset, &(len), sizeof(size_t), 1);
    char* str = (char*) malloc(sizeof(char) * (len + 1));
    offset = read_buffer_from_file(fd, offset, str, sizeof(char), len);
    str[len] = 0;
    *(buffer) = str;
    return offset;
}

int32_t open_file(const char* filename) {
    int32_t fd = open(filename, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);

    if (fd == -1) {
        perror("Cannot open file");
    }

    return fd;
}

void close_file(int32_t fd) {
    close(fd);
}

size_t get_file_len(const int32_t fd) {
    struct stat64 file_stat = {0}; 

    if (fstat64(fd, &file_stat) != 0) {
        perror("Cannot get file stat");
    }

    return file_stat.st_size;
}

void copy_str_to_heap(const char** dest, char* source) {
    *dest = (const char*) malloc(sizeof(char) * (strlen(source) + 1)); // +1 на нуль терминатор
    strcpy((char*) *dest, source);
}
