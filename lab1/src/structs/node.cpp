#include "node.h"

size_t read_first_child_offset(struct file_descriptor* ptr, size_t node_offset) {
    size_t first_child_offset = 0;
    read_buffer_from_file(ptr->fd, node_offset + FIRST_CHILD_OFFSET, &first_child_offset, sizeof(size_t), 1);
    return first_child_offset;
}

size_t read_next_sibiling_offset(struct file_descriptor* ptr, size_t node_offset) {
    size_t next_sibiling_offset = 0;
    read_buffer_from_file(ptr->fd, node_offset + NEXT_SIBILING_OFFSET, &next_sibiling_offset, sizeof(size_t), 1);
    return next_sibiling_offset;    
}

size_t read_prev_sibiling_offset(struct file_descriptor* ptr, size_t node_offset) {
    size_t prev_sibiling_offset = 0;
    read_buffer_from_file(ptr->fd, node_offset + PREV_SIBILING_OFFSET, &prev_sibiling_offset, sizeof(size_t), 1);
    return prev_sibiling_offset; 
}

size_t read_parent_offset(struct file_descriptor* ptr, size_t node_offset){
    size_t parent_offset = 0;
    read_buffer_from_file(ptr->fd, node_offset + PARENT_OFFSET, &parent_offset, sizeof(size_t), 1);
    return parent_offset;    
}

size_t read_node_len(struct file_descriptor* ptr, size_t node_offset) {
    size_t node_len = 0;
    read_buffer_from_file(ptr->fd, node_offset + NODE_ELEM_SIZE_OFFSET, &node_len, sizeof(size_t), 1);
    return node_len;
}

size_t read_node_schema_offset(struct file_descriptor* ptr, size_t node_offset) {
    size_t schema_offset = 0;
    read_buffer_from_file(ptr->fd, node_offset + NODE_SCHEMA_OFFSET, &schema_offset, sizeof(size_t), 1);
    return schema_offset;    
}

struct node* read_node(struct file_descriptor* ptr, size_t offset) {
    struct node* node = (struct node*) malloc(sizeof(struct node));
    node->offset = offset;
    offset = read_buffer_from_file(ptr->fd, offset, &(node->elem_size), sizeof(size_t), 1);
    size_t schema_offset = 0;
    offset = read_buffer_from_file(ptr->fd, offset, &(schema_offset), sizeof(size_t), 1);
    node->schema = read_schema(ptr, schema_offset);
    offset = read_buffer_from_file(ptr->fd, offset, &(node->parent), sizeof(size_t), 1);
    offset = read_buffer_from_file(ptr->fd, offset, &(node->first_child), sizeof(size_t), 1);
    offset = read_buffer_from_file(ptr->fd, offset, &(node->prev_sibiling), sizeof(size_t), 1);
    offset = read_buffer_from_file(ptr->fd, offset, &(node->next_sibiling), sizeof(size_t), 1);

    std::vector<struct attribute*>* attributes = new std::vector<struct attribute*>;
    for (int i = 0; i < node->schema->attributes->size(); i++) {
        struct attribute* attr = (struct attribute*) malloc(sizeof(struct attribute));
        attr->schema = node->schema->attributes->at(i);
        switch (node->schema->attributes->at(i)->type)
        {
        case INT:
            offset = read_buffer_from_file(ptr->fd, offset, &(attr->value.int_value), sizeof(int), 1);
            break;
        case DOUBLE:
            offset = read_buffer_from_file(ptr->fd, offset, &(attr->value.double_value), sizeof(double), 1);
            break;
        case BOOL:
            offset = read_buffer_from_file(ptr->fd, offset, &(attr->value.bool_value), sizeof(bool), 1);
            break;
        case STRING:
            offset = read_null_term_str_from_file(ptr->fd, offset, &(attr->value.string_value));
            break;
        }
        attributes->push_back(attr);
    }
    node->attributes = attributes;

    return node;
}

struct node* read_first_node(struct file_descriptor* ptr) {
    return read_node(ptr, ptr->header->first_node);
}

void write_node(struct file_descriptor* ptr, struct node* node) {
    size_t write_size = NODE_HEADER_SIZE; // плюс длина всех значений атрибутов
    for (int i = 0; i < node->attributes->size(); i++) {
        switch (node->attributes->at(i)->schema->type)
        {
        case INT:
            write_size += sizeof(int);
            break;
        case DOUBLE:
            write_size += sizeof(double);
            break;
        case BOOL:
            write_size += sizeof(bool);
            break;
        case STRING:
            write_size += (strlen(node->attributes->at(i)->value.string_value) + sizeof(size_t)); // добавочек на длину строки
            break;
        }
    }
    if (write_size > node->elem_size) { // нода не помещается в сейчас занимаемую область - вместо неё ставим блок
        if (node->offset != 0) { // если нода не новая
            create_block(node->offset, node->elem_size, ptr);
        }
        node->elem_size = write_size;
        node->offset = find_free_space(ptr->header->first_free_block, node->elem_size, ptr);
        if (node->offset == 0) {
            node->offset = ptr->header->end_file;
            ptr->header->end_file += node->elem_size;
        } else {
            node->elem_size = read_block_size(ptr, node->offset);
            delete_block(node->offset, ptr);
        }
    }
    
    size_t offset = node->offset;
    offset = write_buffer_to_file(ptr->fd, offset, &(node->elem_size), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(node->schema->offset), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(node->parent), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(node->first_child), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(node->prev_sibiling), sizeof(size_t), 1);
    offset = write_buffer_to_file(ptr->fd, offset, &(node->next_sibiling), sizeof(size_t), 1);
    for (int i = 0; i < node->attributes->size(); i++) {
        switch (node->attributes->at(i)->schema->type)
        {
        case INT:
            offset = write_buffer_to_file(ptr->fd, offset, &(node->attributes->at(i)->value.int_value), sizeof(int), 1);
            break;
        case DOUBLE:
            offset = write_buffer_to_file(ptr->fd, offset, &(node->attributes->at(i)->value.double_value), sizeof(double), 1);
            break;
        case BOOL:
            offset = write_buffer_to_file(ptr->fd, offset, &(node->attributes->at(i)->value.bool_value), sizeof(bool), 1);
            break;
        case STRING:
            offset = write_null_term_str_to_file(ptr->fd, offset, node->attributes->at(i)->value.string_value);
            break;
        }
    }
}

void update_first_child(struct file_descriptor* ptr, size_t parent_offset, size_t child_offset) {
    write_buffer_to_file(ptr->fd, parent_offset + FIRST_CHILD_OFFSET, &(child_offset), sizeof(size_t), 1);
}

void update_prev_sibiling(struct file_descriptor* ptr, size_t node_offset, size_t prev_sibiling_offset) {
    write_buffer_to_file(ptr->fd, node_offset + PREV_SIBILING_OFFSET, &(prev_sibiling_offset), sizeof(size_t), 1);
}

void update_next_sibiling(struct file_descriptor* ptr, size_t node_offset, size_t next_sibiling_offset) {
    write_buffer_to_file(ptr->fd, node_offset + NEXT_SIBILING_OFFSET, &(next_sibiling_offset), sizeof(size_t), 1);
}

static void update_parent(struct file_descriptor* ptr, size_t child_offset, size_t parent_offset) {
    write_buffer_to_file(ptr->fd, child_offset + PARENT_OFFSET, &(parent_offset), sizeof(size_t), 1);
}

void update_parent_on_this_and_next_sibiling(struct file_descriptor* ptr, size_t child_offset, size_t parent_offset) {
    while (child_offset != 0) {
        update_parent(ptr, child_offset, parent_offset);
        child_offset = read_next_sibiling_offset(ptr, child_offset);
    }
}

bool compare_not_full_node(struct node* first_node, struct node* second_node) {
    if (first_node->offset != second_node->offset) return false; // эти два должны быть здесь или нет?
    if (first_node->elem_size != second_node->elem_size) return false;
    if (!compare_schema(first_node->schema, second_node->schema)) return false;
    if (first_node->attributes->size() != second_node->attributes->size()) return false;
    for (int i = 0; i < first_node->attributes->size(); i++) {
        switch (first_node->attributes->at(i)->schema->type)
        {
        case INT:
            if (first_node->attributes->at(i)->value.int_value != second_node->attributes->at(i)->value.int_value) return false;
            break;
        case DOUBLE:
            if (first_node->attributes->at(i)->value.double_value != second_node->attributes->at(i)->value.double_value) return false;
            break;
        case BOOL:
            if (first_node->attributes->at(i)->value.bool_value != second_node->attributes->at(i)->value.bool_value) return false;
            break;
        case STRING:
            if (strcmp(first_node->attributes->at(i)->value.string_value, second_node->attributes->at(i)->value.string_value) != 0) return false;
            break;
        }
    }
    return true;
}

bool compare_node(struct node* first_node, struct node* second_node) {
    if (first_node->parent != second_node->parent) return false;
    if (first_node->first_child != second_node->first_child) return false;
    if (first_node->prev_sibiling != second_node->prev_sibiling) return false;
    if (first_node->next_sibiling != second_node->next_sibiling) return false;
    if (!compare_not_full_node(first_node, second_node)) return false;
    return true;
}

bool check_exist_and_update_node(struct file_descriptor* ptr, struct node** node) {
    struct node* real_node = read_node(ptr, (*node)->offset);
    bool result = compare_not_full_node((*node), real_node);
    if (result) {
        (*node)->parent = real_node->parent;
        (*node)->first_child = real_node->first_child;
        (*node)->prev_sibiling = real_node->prev_sibiling;
        (*node)->next_sibiling = real_node->next_sibiling;
    }
    struct schema* schema = real_node->schema;
    free_node(real_node);
    free_schema(schema);
    return result;
}

void free_node(struct node* node) {
    for (int i = 0; i < node->attributes->size(); i++) {
        if (node->attributes->at(i)->schema->type == STRING) {
            delete[] node->attributes->at(i)->value.string_value;
        }
        free(node->attributes->at(i));
    }
    node->attributes->clear();
    delete node->attributes;
    free(node);
}

void replace_node(struct file_descriptor* ptr, size_t new_offset, struct node** node) {
    struct schema* schema = (*node)->schema;
    free_node(*node);
    free_schema(schema);
    *node = read_node(ptr, new_offset);
}

bool check_not_root(struct file_descriptor* ptr, size_t node_offset) {
    return ptr->header->first_node != node_offset;
}
