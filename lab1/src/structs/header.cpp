#include "header.h"

struct file_descriptor* open_file_db(const char* filename) {
    struct file_descriptor* ptr = (struct file_descriptor*) malloc(sizeof(struct file_descriptor));
    int32_t fd = open_file(filename);
    ptr->fd = fd;
    size_t size = get_file_len(fd);
    if (size <= 1) {
        // файл новый, только создался, логика по заполнению хэдера и потом его запись
    } else {
        // файл уже существовал, надо только вычитать хедер
        ptr->header = read_header(fd);
    }
    return ptr;
}

void close_file_db(struct file_descriptor* ptr) {
    write_header(ptr);
    close_file(ptr->fd);
    free(ptr->header);
    free(ptr);
    ptr = NULL;
}

struct tree_header* read_header(int32_t fd) {
    return NULL;
}

void write_header(struct file_descriptor* ptr) {
    
}
