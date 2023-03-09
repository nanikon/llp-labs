#include "block.h"

void free_block(struct block* block) {
    free(block);
}

void print_block(struct block* block){
	std::cout << "next:" << block->next;
	std::cout << ", prev:" << block->prev;
	std::cout << ", size:" << block->size;
}

void write_block(struct block* block, struct file_descriptor* ptr, size_t offset) {
    offset = write_buffer_to_file(ptr->fd, offset, &(block->next), 1, sizeof(size_t));
	offset = write_buffer_to_file(ptr->fd, offset, &(block->prev), 1, sizeof(size_t));
	offset = write_buffer_to_file(ptr->fd, offset, &(block->size), 1, sizeof(size_t));
}

void write_block(size_t next, size_t prev, size_t size, struct file_descriptor* ptr, size_t offset) {
    offset = write_buffer_to_file(ptr->fd, offset, &next, 1, sizeof(size_t));
	offset = write_buffer_to_file(ptr->fd, offset, &prev, 1, sizeof(size_t));
	offset = write_buffer_to_file(ptr->fd, offset, &size, 1, sizeof(size_t));
}

struct block* read_block(struct file_descriptor* ptr, size_t offset) {
    struct block* block = (struct block*) malloc(sizeof(struct block));
    offset = read_buffer_from_file(ptr->fd, offset, &(block->next), 1, sizeof(size_t));
	offset = read_buffer_from_file(ptr->fd, offset, &(block->prev), 1, sizeof(size_t));
	offset = read_buffer_from_file(ptr->fd, offset, &(block->size), 1, sizeof(size_t));
	return block;
}

size_t read_block_size(struct file_descriptor* ptr, size_t offset) {
    size_t block_size = 0;
	read_buffer_from_file(ptr->fd, offset + BLOCK_SIZE_OFFSET, &block_size, 1, sizeof(size_t));
	return block_size;
}

size_t read_block_next(struct file_descriptor* ptr, size_t offset) {
    size_t block_next = 0;
	read_buffer_from_file(ptr->fd, offset + BLOCK_NEXT_OFFSET, &block_next, 1, sizeof(size_t));
	return block_next;
}

size_t read_block_prev(struct file_descriptor* ptr, size_t offset) {
 	size_t block_prev = 0;
	read_buffer_from_file(ptr->fd, offset + BLOCK_PREV_OFFSET, &block_prev, 1, sizeof(size_t));
	return block_prev;
}

void update_block_prev(struct file_descriptor* ptr, size_t offset, size_t new_prev_offset) {
    write_buffer_to_file(ptr->fd, offset + BLOCK_PREV_OFFSET, &new_prev_offset, 1, sizeof(size_t));
}

void update_block_next(struct file_descriptor* ptr, size_t offset, size_t new_next_offset) {
 	write_buffer_to_file(ptr-> fd, offset + BLOCK_NEXT_OFFSET, &new_next_offset, 1, sizeof(size_t));
}

void delete_block(size_t offset, size_t* tree_head_offset, struct file_descriptor* ptr) {
    size_t next = read_block_next(ptr, offset);
	size_t prev = read_block_prev(ptr, offset);
	
	if (prev == 0) { 
	    *tree_head_offset = next;
	    update_block_prev(ptr, next, 0);
	} else if (next == 0) { 
	    update_block_next(ptr, prev, 0);
	} else {
	    update_block_next(ptr, prev, next);
	    update_block_prev(ptr, next, prev);
    }
}

void create_block(size_t* free_space_offset, size_t block_offset, size_t block_size, struct file_descriptor* ptr) {
	update_block_prev(ptr, *free_space_offset, block_offset);
	write_block(*free_space_offset, 0, block_size, ptr, block_offset);
	*free_space_offset = block_offset;
}

size_t find_free_space(size_t block, size_t size, struct file_descriptor* ptr) {
    size_t next;
	do {
	    next = read_block_next(ptr, block);
	    if (read_block_size(ptr, block) >= size) return block;
	    else block = next;
	} while(next != 0);
	return 0;
}

void print_free_space(size_t free_space_offset, struct file_descriptor* ptr) {
    size_t next;
	size_t block = free_space_offset;
	do {
	    std::cout << "offset:" << block << ", size:" << read_block_size(ptr, block);
	    std::cout << ", next:" << read_block_next(ptr, block)<< ", prev:" << read_block_prev(ptr, block) << std::endl;
	    next = read_block_next(ptr, block);
	    block = next;
	} while(next != 0);
}