#ifndef LLP_HEADER_H
#define LLP_HEADER_H

#include <stdio.h>
#include <stdint.h>

#define START_OFFSET 0
#define FILE_HEADER_SIZE sizeof(size_t)*3
#define TREE_HEAD_OFFSET 0
#define FREE_SPACE_OFFSET sizeof(size_t)
#define END_OF_LAST_NODE_OFFSET sizeof(size_t)*2

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
    int32_t fd;
    struct tree_header header;
};

struct file_descriptor open_file(const char* filename);
void close_file(struct file_descriptor fd);

struct tree_header* read_header(struct file_descriptor* ptr);
size_t read_tree_head_offset(struct file_descriptor* ptr);
size_t read_free_space_offset(struct file_descriptor* ptr);
size_t read_end_of_last_node_offset(struct file_descriptor* ptr);

void write_header(struct file_descriptor* ptr);
void write_tree_head_offset(size_t offset, struct file_descriptor* ptr);
void write_free_space_offset(size_t offset, struct file_descriptor* ptr);
void write_end_of_last_node_offset(size_t offset, struct file_descriptor* ptr);

void print_header(struct file_descriptor* ptr);

#endif 
