#include "schema.h"

void write_schema(struct file_descriptor* ptr, struct schema* schema) {
    size_t name_len = strlen(schema->name);
    size_t attr_count = schema->attributes->size();
    size_t attr_len = 0;
    for (int i = 0; i < schema->attributes->size(); i++) {
        attr_len += ATTRIBUTE_SCHEMA_SIZE + strlen(schema->attributes->at(i)->name); // size_t на длину имени берется из ATTRIBUTE_SCHEMA_SIZE вместо char*
    }
    size_t write_size = STRUCT_SCHEMA_HEADER_SIZE + name_len + attr_len; // size_t на длину имени берется из STRUCT_SCHEMA_HEADER_SIZE
    schema->elem_size = write_size;
    schema->offset = find_free_space(ptr->header->first_free_block, write_size, ptr);
    if (schema->offset == 0) {
        schema->offset = ptr->header->end_file;
        ptr->header->end_file += write_size;
    } else {
        schema->elem_size = read_block_size(ptr, schema->offset);
        delete_block(schema->offset, ptr); 
    }

    update_schema_next(ptr, ptr->header->last_schema, schema->offset);

    size_t offset = schema->offset;
    offset = write_buffer_to_file(ptr->fd, offset, &(attr_count), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->elem_size), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->next), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(schema->count), sizeof(size_t), 1);
    offset = write_null_term_str_to_file(ptr->fd, offset, schema->name);
    for (int i = 0; i < schema->attributes->size(); i++) {
        offset = write_buffer_to_file(ptr->fd, offset, &(schema->attributes->at(i)->type), sizeof(enum value_type), 1);
        offset = write_null_term_str_to_file(ptr->fd, offset, schema->attributes->at(i)->name);
    }
}

void update_schema_next(struct file_descriptor* ptr, size_t schema_offset, size_t next) {
    write_buffer_to_file(ptr->fd, schema_offset + SCHEMA_NEXT_OFFSET, &(next), sizeof(size_t), 1);
}

void update_schema_count(struct file_descriptor* ptr, size_t schema_offset, size_t count) {
    write_buffer_to_file(ptr->fd, schema_offset + SCHEMA_COUNT_OFFSET, &count, sizeof(size_t), 1);
}

struct schema* read_first_schema(struct file_descriptor* ptr) {
    size_t first_schema_offset = read_schema_next(ptr, ptr->header->first_schema);
    if (first_schema_offset == NULL) {
        return NULL;
    }
    return read_schema(ptr, first_schema_offset);
}

size_t read_schema_next(struct file_descriptor* ptr, size_t schema_offset) {
    size_t next = 0;
    read_buffer_from_file(ptr->fd, schema_offset + SCHEMA_NEXT_OFFSET, &next, sizeof(size_t), 1);
    return next;
}

size_t read_schema_count(struct file_descriptor* ptr, size_t schema_offset) {
    size_t count = 0;
    read_buffer_from_file(ptr->fd, schema_offset + SCHEMA_COUNT_OFFSET, &count, sizeof(size_t), 1);
    return count;
}

struct schema* read_schema(struct file_descriptor* ptr, size_t offset) {
    size_t schema_offset = offset;
    size_t attribute_count = 0;
    offset = read_buffer_from_file(ptr->fd, offset, &attribute_count, sizeof(size_t), 1);
    size_t elem_size = 0;
    offset = read_buffer_from_file(ptr->fd, offset, &elem_size, sizeof(size_t), 1);

    struct schema* schema = (struct schema*) malloc(sizeof(struct schema)); // добавочек - на нуль-терминаторы для строк
    schema->elem_size = elem_size;
    schema->offset = schema_offset;

    offset = read_buffer_from_file(ptr->fd, offset, &(schema->next), sizeof(size_t), 1);
    offset = read_buffer_from_file(ptr->fd, offset, &(schema->count), sizeof(size_t), 1);  
    offset = read_null_term_str_from_file(ptr->fd, offset, &(schema->name));

    std::vector<struct attribute_schema*>* attributes = new std::vector<struct attribute_schema*>;
    for (int i = 0; i < attribute_count; i++) {
        struct attribute_schema* attr = (struct attribute_schema*) malloc(sizeof(struct attribute_schema));
        offset = read_buffer_from_file(ptr->fd, offset, &(attr->type), sizeof(enum value_type), 1);
        offset = read_null_term_str_from_file(ptr->fd, offset, &(attr->name));
        attributes->push_back(attr);
    }
    schema->attributes = attributes;
    return schema;
}

void free_schema(struct schema* schema) {
    for (int i = 0; i < schema->attributes->size(); i++) {
        delete[] schema->attributes->at(i)->name;
    }
    free(schema->attributes);
    delete[] schema->name;
    free(schema);
}

bool compare_schema(struct schema* first_schema, struct schema* second_schema) {
    if (first_schema->offset != second_schema->offset) return false;
    if (first_schema->elem_size != second_schema->elem_size) return false;
    if (first_schema->next != second_schema->next) return false;
    if (first_schema->count != second_schema->count) return false;
    if (strcmp(first_schema->name, second_schema->name) != 0) return false;
    if (first_schema->attributes->size() != second_schema->attributes->size()) return false;
    for (int i = 0; i < first_schema->attributes->size(); i++) {
        if (first_schema->attributes->at(i)->type != second_schema->attributes->at(i)->type) return false;
        if (strcmp(first_schema->attributes->at(i)->name, second_schema->attributes->at(i)->name) != 0) return false;
    }
    return true;
}

bool check_exist_schema(struct file_descriptor* ptr, struct schema* schema) {
    struct schema* real_schema = read_schema(ptr, schema->offset);
    bool result = compare_schema(schema, real_schema);
    free_schema(real_schema);
    return result;
}
