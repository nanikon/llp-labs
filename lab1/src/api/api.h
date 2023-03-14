#ifndef LLP_API_H
#define LLP_API_H

#include <vector>
#include <string>
#include <stdlib.h>

#include "../structs/schema.h"
#include "../structs/block.h"

//file

//schema 
enum schema_operation_status {
    OK,
    HAS_DEPENDS
};

struct schema* create_schema(struct file_descriptor* ptr, char* name, std::vector<struct attribute_schema*>* attributes);
enum schema_operation_status delete_schema(struct file_descriptor* ptr, struct schema* schema);

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

// node

// search

#endif