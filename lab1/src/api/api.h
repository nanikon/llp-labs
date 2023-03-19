#ifndef LLP_API_H
#define LLP_API_H

#include <vector>
#include <string>
#include <stdlib.h>

#include "../structs/schema.h"
#include "../structs/block.h"
#include "../structs/node.h"

//file

//schema 

enum schema_delete_operation_status {
    OK_SCHEMA_DELETE = 0,
    HAS_DEPENDS,
    NOT_FOUND_ON_FILE
};

struct schema* create_schema(struct file_descriptor* ptr, char* name, std::vector<struct attribute_schema*>* attributes);
enum schema_delete_operation_status delete_schema(struct file_descriptor* ptr, struct schema* schema);

typedef struct Schema_Iter {
    struct schema* schema;
    struct file_descriptor* ptr;
    bool is_valid = true;

    Schema_Iter(struct file_descriptor* ptr) : ptr(ptr) {
        this->schema = read_first_schema(ptr);
        if (this->schema == NULL) {
            is_valid = false;
        }
    }

    bool next();
    struct schema* operator*();
    void free();
} Schema_Iter;

Schema_Iter read_schemas(struct file_descriptor* ptr);

// node - create, update, delete

enum node_create_operation_status {
    OK_NODE_CREATE = 0,
    SCHEMA_NOT_FOUND_ON_FILE,
    PARENT_NOT_FOUND_ON_FILE,
    WROND_ATTRIBUTE_NODE_CREATE,
    ATTRIBUTE_NOT_FOUND
};
enum node_create_operation_status create_node(struct file_descriptor* ptr, struct schema* schema, struct node* parent, std::vector<struct attribute*>* attributes, struct node** node);

enum node_update_operation_status {
    OK_NODE_UPDATE = 0,
    NODE_NOT_FOUND_ON_FILE_NODE_UPDATE,
    WRONG_ATTRIBUTE_NOTE_UPDATE
};
/**
 * @param attributes - обновляются только те аттрибуты ноды, которые есть в этом списке
*/
enum node_update_operation_status update_node(struct file_descriptor* ptr, struct node* node, std::vector<struct attribute*>* attributes);

enum node_delete_operation_status {
    OK_NODE_DELETE = 0,
    NODE_NOT_FOUND_ON_FILE_NODE_DELETE
};
enum node_delete_operation_status delete_node(struct file_descriptor* ptr, struct node* node);

// node - search

#endif