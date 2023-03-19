#include "api.h"

struct schema* Schema_Iter::operator*() {
    return schema;
}

bool Schema_Iter::next() {
    if (this->schema->next == 0) {
        is_valid = false;
        return false;
    }
    this->schema = read_schema(this->ptr, this->schema->next);
    return true;
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
    schema->next = 0;
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
