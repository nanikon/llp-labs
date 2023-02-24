#ifndef LLP_HEADER_H
#define LLP_HEADER_H

#include <stdio.h>
#include <stdint.h>

struct tree_header {
    // Смещение первого узла дерева относительно начала файла
    int64_t first_node;
    // Смещение первой схемы узла относительно начала файла
    int64_t first_schema;
    // Смещение первого свободного блока относительно начала файла
    int64_t first_free_block;
    // у Фарида есть ещё size, у Маши - смещение конца файла
};

// tree_header лежит внутри, кажется что пользователь не должен иметь к нему доступ
struct file_descriptor {
    FILE* file;
    struct tree_header header;
};

struct file_descriptor open_file(const char* filename);

void close_file(struct file_descriptor fd);

#endif 
