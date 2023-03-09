#ifndef LLP_BLOCK_H
#define LLP_BLOCK_H

#include <stdint.h>
#include <iostream>
#include <stdlib.h>
#include "../file/file.h"
#include "header.h"

#define BLOCK_SIZE sizeof(size_t) * 3
#define DEFAULT_BLOCK_SIZE BLOCK_SIZE
#define BLOCK_SIZE_OFFSET sizeof(size_t) * 2
#define BLOCK_PREV_OFFSET sizeof(size_t)
#define BLOCK_NEXT_OFFSET 0

struct block {
    size_t next;
    size_t prev;
    size_t size;
};

void free_block(struct block* block);
void print_block(struct block* block);

void write_block(struct block* block, struct file_descriptor* ptr, size_t offset);
void write_block(size_t next, size_t prev, size_t size, struct file_descriptor* ptr, size_t offset);

struct block* read_block(struct file_descriptor* ptr, size_t offset);
size_t read_block_size(struct file_descriptor* ptr, size_t offset);
size_t read_block_next(struct file_descriptor* ptr, size_t offset);
size_t read_block_prev(struct file_descriptor* ptr, size_t offset);

void update_block_prev(struct file_descriptor* ptr, size_t offset, size_t new_prev_offset);
void update_block_next(struct file_descriptor* ptr, size_t offset, size_t new_next_offset);

void delete_block(size_t offset, size_t* tree_head_offset, struct file_descriptor* ptr);
void create_block(size_t* free_space_offset, size_t block_offset, size_t block_size, struct file_descriptor* ptr);

size_t find_free_space(size_t block, size_t size, struct file_descriptor* ptr);
void print_free_space(size_t free_space_offset, struct file_descriptor* ptr); // отладочная, потом убрать

#endif
