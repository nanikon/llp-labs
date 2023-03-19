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

struct attribute_schema* create_attribute(enum value_type type, char* name) {
    struct attribute_schema* attr = (struct attribute_schema*) malloc(sizeof(struct attribute_schema));
    attr->type = type;
    attr->name = name;
    return attr;
}

struct schema* create_schema(struct file_descriptor* ptr, char* name, std::vector<struct attribute_schema*>* attributes) {
    struct schema* schema = (struct schema*) calloc(1, sizeof(struct schema));
    schema->attributes = attributes;
    schema->name = name;
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

static struct attribute* create_attribute(struct attribute_schema* schema, union data value) {
    struct attribute* attr = (struct attribute*) calloc(1, sizeof(struct attribute));
    attr->schema = schema;
    attr->value = value;
    return attr;
}

enum node_create_operation_status create_node(
    struct file_descriptor* ptr, 
    struct schema* schema, 
    struct node* parent, 
    std::unordered_map<struct attribute_schema*, union data> attributes,
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

    std::vector<struct attribute*>* real_attributes = new std::vector<struct attribute*>;
    for (int i = 0; i < schema->attributes->size(); i++) {
        if (!attributes.contains(schema->attributes->at(i))) {
            return ATTRIBUTE_NOT_FOUND;
        }
        real_attributes->push_back(create_attribute(schema->attributes->at(i), attributes[schema->attributes->at(i)]));
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
    update_prev_sibiling(ptr, parent->first_child, node->offset);
    parent->first_child = (*node)->offset;
    update_first_child(ptr, parent->offset, node->offset);
    
    return OK_NODE_CREATE;
}

enum node_update_operation_status update_node(struct file_descriptor* ptr, struct node* node, std::unordered_map<struct attribute_schema*, union data> attributes) {
    if (!check_exist_node(ptr, node)) {
        return NODE_NOT_FOUND_ON_FILE_NODE_UPDATE;
    }
    size_t changed_attrs = 0;
    std::vector<struct attribute*>* new_attributes = new std::vector<struct attribute*>;
    for (int i = 0; i < node->attributes->size(); i++) {
        if (attributes.contains(node->attributes->at(i)->schema)) {
            new_attributes->push_back(create_attribute(node->attributes->at(i)->schema, attributes[node->attributes->at(i)->schema]));
            changed_attrs++;
        } else {
            new_attributes->push_back(node->attributes->at(i));
        }
    }
    if (changed_attrs != attributes.size()) {
        return WRONG_ATTRIBUTE_NOTE_UPDATE;
    }
    free(node->attributes);
    node->attributes = new_attributes;

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
