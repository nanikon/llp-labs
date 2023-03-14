#include "schema.h"

void write_schema(struct file_descriptor* ptr, struct schema* schema) {
    size_t name_len = strlen(schema->name);
    size_t attributes_len = 0;
    for (int i = 0; i < schema->attributes->size; i++) {
        attributes_len += sizeof(struct attribute) + strlen(schema->attributes[i]->name);
    }
    size_t write_size = STRUCT_SHEMA_HEADER_SIZE + name_len + attributes_len;
    schema->elem_size = write_size;
    schema->offset = find_free_space(ptr->header->first_free_block, write_size, ptr);
    if (schema->offset == 0) {
        schema->offset = ptr->header->end_file;
        ptr->header->end_file += schema_write_size;
    } else {
        schema->elem_size = read_block_size(ptr, schema->offset);
        delete_block(schema->offset, ptr); 
    }

    update_schema_next(ptr, ptr->header->last_schema, schema->offset);

    size_t offset = schema->offset;
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->attributes->size), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->elem_size), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->next), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->count), sizeof(size_t), 1);
    offset = write_str_with_len_to_file(ptr->fd, offset, schema->name);
    for (int i = 0; i < schema->attributes->size; i++) {
        offset = write_buffer_to_file(ptr->fd, offset, schema->attributes[i]->type, sizeof(enum type), 1);
        offset = write_str_with_len_to_file(ptr->fd, schema->attributes[i]->name);
    }
}