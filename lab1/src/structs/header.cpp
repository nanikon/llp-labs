#include "header.h"

struct tree_header* read_header(int32_t fd) {
    struct tree_header* header = (struct tree_header*) malloc(sizeof(struct tree_header));
    size_t offset = read_buffer_from_file(fd, 0, &(header->first_node), sizeof(size_t), 1);
    offset = read_buffer_from_file(fd, offset, &(header->first_schema), sizeof(size_t), 1);
    offset = read_buffer_from_file(fd, offset, &(header->last_schema), sizeof(size_t), 1);
    offset = read_buffer_from_file(fd, offset, &(header->first_free_block), sizeof(size_t), 1);
    offset = read_buffer_from_file(fd, offset, &(header->end_file), sizeof(size_t), 1);
    return header;
}

void write_header(struct file_descriptor* ptr) {
    size_t offset = write_buffer_to_file(ptr->fd, 0, &(ptr->header->first_node), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(ptr->header->first_schema), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(ptr->header->last_schema), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(ptr->header->first_free_block), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(ptr->header->end_file), sizeof(size_t), 1);
}
