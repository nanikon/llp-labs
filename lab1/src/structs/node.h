#ifndef LLP_NODE_H
#define LLP_NODE_H

#include "schema.h"

union data {
    int int_value;
    double double_value;
    const char* string_value;
    bool bool_value;
};

struct attribute {
    struct attribute_schema* schema;
    union data value; 
};

#define FIRST_CHILD_OFFSET sizeof(size_t) * 3
#define PREV_SIBILING_OFFSET sizeof(size_t) * 4
#define NEXT_SIBILING_OFFSET sizeof(size_t) * 5
#define NODE_ELEM_SIZE_OFFSET 0
#define NODE_SCHEMA_OFFSET sizeof(size_t)
#define NODE_HEADER_SIZE sizeof(size_t) * 6
struct node {
    size_t offset;
    size_t elem_size;
    struct schema* schema;

    size_t parent;
    size_t first_child;
    size_t prev_sibiling;
    size_t next_sibiling;

    std::vector<struct attribute*>* attributes;
};
/* Как нода хранится в файле
    - длина блока в котором записан
    - смещение схемы
    - смещение родителя
    - смещение первого ребенка
    - смещение предыдущего сибилинга
    - смещение следующего сибилинга
    далее идет список аттрибутов (их количество берется из схемы):
        - значение аттрибута. Тип берется из схемы аттрибута. Если int, double, bool - то так и записаны, если char* - то длина + сама строка (идентификатор аттрибута не нужен, т.к. полагаемся на порядок)
*/

struct node* read_node(struct file_descriptor* ptr, size_t offset);
size_t read_first_child_offset(struct file_descriptor* ptr, size_t node_offset);
size_t read_next_sibiling_offset(struct file_descriptor* ptr, size_t node_offset);
size_t read_node_len(struct file_descriptor* ptr, size_t node_offset);
size_t read_node_schema_offset(struct file_descriptor* ptr, size_t node_offset);

// должна быть универсальной и использоваться как при обновлении, так и при создании
void write_node(struct file_descriptor* ptr, struct node* node);
void update_first_child(struct file_descriptor* ptr, size_t parent_offset, size_t child_offset);
void update_prev_sibiling(struct file_descriptor* ptr, size_t node_offset, size_t prev_sibiling_offset);
void update_next_sibiling(struct file_descriptor* ptr, size_t node_offset, size_t next_sibiling_offset);

bool check_exist_node(struct file_descriptor* ptr, struct node* node);

void free_node(struct node* node);

#endif