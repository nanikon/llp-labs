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

enum schema_operation_status delete_schema(struct file_descriptor* ptr, struct schema* schema) {
    if (schema->count > 0) {
        return HAS_DEPENDS;
    }
    create_block(schema->offset, schema->elem_size, ptr);
    free_schema(schema);
    return OK;
}