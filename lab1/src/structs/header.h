#ifndef LLP_HEADER_H
#define LLP_HEADER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../file/file.h"

#define START_OFFSET 0
#define FILE_HEADER_SIZE sizeof(size_t) * 5
#define TREE_HEAD_OFFSET 0
#define SHEMA_OFFSET sizeof(size_t)
#define FREE_SPACE_OFFSET sizeof(size_t) * 3
#define END_OF_LAST_NODE_OFFSET sizeof(size_t) * 4
struct tree_header {
    // Смещение первого узла дерева относительно начала файла
    size_t first_node;
    // Смещение первой схемы узла относительно начала файла
    size_t first_schema;
    // Смещение последней схемы узла относительно начала файла
    size_t last_schema;
    // Смещение первого свободного блока относительно начала файла
    size_t first_free_block;
    // Конец файла - сюда идет запись если в нем самом нет пустого места
    size_t end_file;
};

// tree_header лежит внутри, кажется что пользователь не должен иметь к нему доступ
struct file_descriptor {
    int32_t fd;
    struct tree_header* header;
};

struct tree_header* read_header(int32_t fd); 

void write_header(struct file_descriptor* ptr);

void print_header(struct file_descriptor* ptr);

#endif 
