#include "api.h"

struct schema* Schema_Iter::operator*() {
    return schema;
}

bool Schema_Iter::next() {
    if (this->schema->next == NULL) {
        is_valid = false;
        return false;
    } else {
        this->schema = read_schema(this->ptr, this->schema->next);
    }
}

void Schema_Iter::free() {
    free_schema(this->schema);
}

Schema_Iter read_schemas(struct file_descriptor* ptr) {
    return Schema_Iter(ptr);
}

struct schema* create_schema(struct file_descriptor* ptr, char* name, std::vector<struct attribute_schema*>* attributes) {
    struct schema* schema = (struct schema*) malloc(sizeof(struct schema));
    schema->name = name;
    schema->attributes = attributes;
    schema->count = 0;
    schema->next = NULL;
    write_schema(ptr, schema);
    return schema;
}

enum schema_delete_operation_status delete_schema(struct file_descriptor* ptr, struct schema* schema) {
    schema->count = read_schema_count(ptr, schema->offset); // подобновляем на случай если удалялось поддерево с такими элементами
    if (schema->count > 0) {
        return HAS_DEPENDS;
    }
    if (!check_exist_schema(ptr, schema)) {
        return NOT_FOUND_ON_FILE;
    }
    create_block(schema->offset, schema->elem_size, ptr);
    free_schema(schema);
    return OK_SCHEMA_DELETE;
}

enum node_create_operation_status create_node(
    struct file_descriptor* ptr, 
    struct schema* schema, 
    struct node* parent, 
    std::vector<struct attribute*>* attributes, 
    struct node** node
) {
    if (!check_exist_schema(ptr, schema)) {
        return SCHEMA_NOT_FOUND_ON_FILE;
    }
    if (parent == NULL) {
        parent = read_node(ptr, ptr->header->first_node);
    } else if (!check_exist_node(ptr, parent)) {
        return PARENT_NOT_FOUND_ON_FILE;
    }
    for (int i = 0; i < schema->attributes->size(); i++) {
        bool is_found = false;
        for (int j = 0; j < attributes->size(); j++) {
            if ((schema->attributes->at(i)->type == attributes->at(j)->schema->type) && (strcmp(schema->attributes->at(i)->name, attributes->at(j)->schema->name) == 0)) {
                is_found = true;
                break;
            }
        }
        if (!is_found) {
            return ATTRIBUTE_NOT_FOUND;
        }
    }
    if (schema->attributes->size() != attributes->size()) return WROND_ATTRIBUTE_NODE_CREATE;

    *node = (struct node*) calloc(1, sizeof(struct node));
    (*node)->schema = schema;
    (*node)->parent = parent->offset;
    (*node)->next_sibiling = parent->first_child;
    (*node)->attributes = attributes;
    
    schema->count++;
    update_schema_count(ptr, schema->offset, schema->count);
    write_node(ptr, *node);
    
    return OK_NODE_CREATE;
}

enum node_update_operation_status update_node(struct file_descriptor* ptr, struct node* node, std::vector<struct attribute*>* attributes) {
    if (!check_exist_node(ptr, node)) {
        return NODE_NOT_FOUND_ON_FILE_NODE_UPDATE;
    }
    for (int i = 0; i < attributes->size(); i++) {
        bool is_found = false;
        for (int j = 0; j < node->attributes->size(); j++) {
            if ((attributes->at(i)->schema->type == node->attributes->at(j)->schema->type) && (strcmp(attributes->at(i)->schema->name, node->attributes->at(j)->schema->name) == 0)) {
                is_found = true;
                node->attributes->at(j)->value = attributes->at(i)->value;
                break;
            }
        }
        if (!is_found) {
            return WRONG_ATTRIBUTE_NOTE_UPDATE;
        }
    }
    // create_block() - ? здесь или не здесь

    write_node(ptr, node);

    return OK_NODE_UPDATE;
}

void delete_node_with_children_and_next_sibilings(struct file_descriptor* ptr, size_t node_offset) {
    size_t first_child = read_first_child_offset(ptr, node_offset);
    if( first_child != 0) { 
        delete_node_with_children_and_next_sibilings(ptr, first_child);
    }

    size_t next_sibling = read_next_sibiling_offset(ptr, node_offset);
    if (next_sibling != 0) { 
        delete_node_with_children_and_next_sibilings(ptr, next_sibling);
    }
    
    size_t schema_offset = read_node_schema_offset(ptr, node_offset);
    size_t count = read_schema_count(ptr, schema_offset);
    update_schema_count(ptr, schema_offset, count - 1);
    create_block(&(ptr->header->first_free_block), node_offset, read_node_len(ptr, node_offset), ptr);
}

enum node_delete_operation_status delete_node(struct file_descriptor* ptr, struct node* node) {
    if (!check_exist_node(ptr, node)) {
        return NODE_NOT_FOUND_ON_FILE_NODE_DELETE;
    }
    if (node->first_child != 0) {
        delete_node_with_children_and_next_sibilings(ptr, node->first_child);
    }

    // удалить себя у родителей и братьев
    if(node->prev_sibiling == 0){
        if(node->next_sibiling == 0){
            update_first_child(ptr, node->parent, 0);
        }else{
            update_first_child(ptr, node->parent, node->next_sibiling);
            update_prev_sibiling(ptr, node->next_sibiling, 0);
        }
    }else if(node->next_sibiling != 0) {
        update_prev_sibiling(ptr, node->next_sibiling, node->prev_sibiling);
        update_next_sibiling(ptr, node->prev_sibiling, node->next_sibiling);
    } else { 
        update_next_sibiling(ptr, node->prev_sibiling, 0); // изменила с update_prev_sibiling, возможно не надо было, проверить тестом
    }

    node->schema->count--;
    update_schema_count(ptr, node->schema->offset, node->schema->count);
    create_block(node->offset, node->elem_size, ptr);
    free(node);
}
