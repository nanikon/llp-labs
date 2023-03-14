#ifndef LLP_HEADER_H
#define LLP_HEADER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../file/file.h"

#define START_OFFSET 0
#define FILE_HEADER_SIZE sizeof(size_t) * 3
#define TREE_HEAD_OFFSET 0
#define SHEMA_OFFSET sizeof(size_t)
#define FREE_SPACE_OFFSET sizeof(size_t) * 2
#define END_OF_LAST_NODE_OFFSET sizeof(size_t) * 2

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

struct file_descriptor* open_file_db(const char* filename);
void close_file_db(struct file_descriptor* ptr);

struct tree_header* read_header(int32_t fd); // подумать что передавать - file_descriptor или число напрямую
size_t read_tree_head_offset(int32_t fd);
size_t read_free_space_offset(int32_t fd);
size_t read_end_of_last_node_offset(int32_t fd);

void write_header(struct file_descriptor* ptr);
void write_tree_head_offset(size_t offset, struct file_descriptor* ptr);
void write_free_space_offset(size_t offset, struct file_descriptor* ptr);
void write_end_of_last_node_offset(size_t offset, struct file_descriptor* ptr);

void print_header(struct file_descriptor* ptr);

#endif 
