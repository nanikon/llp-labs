#ifndef LLP_API_H
#define LLP_API_H

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include <map>
#include <stdlib.h>

#include "../structs/schema.h"
#include "../structs/block.h"
#include "../structs/node.h"

//file

struct file_descriptor* open_file_db(const char* filename);
void close_file_db(struct file_descriptor* ptr);

//schema 

enum schema_delete_operation_status {
    OK_SCHEMA_DELETE = 0,
    HAS_DEPENDS,
    NOT_FOUND_ON_FILE
};

/** @param name - для объекта копируется в кучу, основной экземпляр должен управляются пользователем */
struct attribute_schema* create_attribute(enum value_type type, char* name);
/** @param attributes - для объекта копируется в кучу, основной экземпляр должен управляются пользователем  */
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
/** @param attributes - ключи мапы должны быть из schema, иначе не будут распознаны. Запись аттрибутов будет в том порядке, в котором они указаны в схеме
 *  @param parent - родитель вершины, если хотим сделать корневой, то надо передать null
 */
enum node_create_operation_status create_node(
    struct file_descriptor* ptr, 
    struct schema* schema, 
    struct node* parent, 
    std::tr1::unordered_map<struct attribute_schema*, union data> attributes,
    struct node** node
);

enum node_update_operation_status {
    OK_NODE_UPDATE = 0,
    NODE_NOT_FOUND_ON_FILE_NODE_UPDATE,
    WRONG_ATTRIBUTE_NOTE_UPDATE
};
/**
 * @param attributes - обновляются только те аттрибуты ноды, которые есть в этой мапе, ключи должны быть из node->schema, иначе не будут распознаны
*/
enum node_update_operation_status update_node(struct file_descriptor* ptr, struct node* node, std::tr1::unordered_map<struct attribute_schema*, union data> attributes);

enum node_delete_operation_status {
    OK_NODE_DELETE = 0,
    NODE_NOT_FOUND_ON_FILE_NODE_DELETE
};
enum node_delete_operation_status delete_node(struct file_descriptor* ptr, struct node* node);

// node - search

enum digit_operations {
    DIGIT_EQUAL,
    DIGIT_NOT_EQUAL,
    DIGIT_LESS,
    DIGIT_LESS_OR_EQUAL,
    DIGIT_GREAT_OR_EQUAL,
    DIGIT_GREAT
};

enum bool_operations {
    BOOL_EQUAL,
    BOOL_NOT_EQUAL
};

enum string_operations {
    STRING_EQUAL,
    STRING_NOT_EQUAL,
    STRING_CONTAINS
};

struct int_condition {
    int value;
    enum digit_operations operation;
};

struct double_condition {
    double value;
    enum digit_operations operation;
};

struct bool_condition {
    bool value;
    enum bool_operations operation;
};

struct string_condition {
    char* value;
    enum string_operations operation;
};

struct attr_condition {
    enum value_type type;
    char* name;
    union {
        struct int_condition* int_c;
        struct double_condition* double_c;
        struct bool_condition* bool_c;
        struct string_condition* string_c;
    };
};

struct schema_condition {
    struct schema* schema;
};

enum node_condition_type {
    ATTR,
    SCHEMA
};

struct node_condition {
    int8_t node_order; // понадеюсь что более глубоких не будет
    enum node_condition_type type;
    union {
        struct attr_condition* attr_c;
        struct schema_condition* schema_c;
    };
};

enum condition_type {
    OR,
    AND,
    NOT,
    NODE
};

struct search_node {
    enum condition_type type;
    union {
        struct and_condition* and_c;
        struct or_condition* or_c;
        struct not_condition* not_c;
        struct node_condition* node_c;
    };
};

struct and_condition {
    struct search_node* a;
    struct search_node* b;
};

struct or_condition {
    struct search_node* a;
    struct search_node* b;
};

struct not_condition {
    struct search_node* a;
};

/**
 * @param attr_name - в структуру копируется, поэтому при необходимости пользователь исходник должен очистить сам
*/
struct search_node* make_int_attr_cond(int8_t node_order, char* attr_name, enum digit_operations operation, int value);
/**
 * @param attr_name - в структуру копируется, поэтому при необходимости пользователь исходник должен очистить сам
*/
struct search_node* make_double_attr_cond(int8_t node_order, char* attr_name, enum digit_operations operation, double value);
/**
 * @param attr_name - в структуру копируется, поэтому при необходимости пользователь исходник должен очистить сам
*/
struct search_node* make_bool_attr_cond(int8_t node_order, char* attr_name, enum bool_operations operation, bool value);
/**
 * @param attr_name - в структуру копируется, поэтому при необходимости пользователь исходник должен очистить сам
 * @param value - в структуру копируется, поэтому при необходимости пользователь исходник должен очистить сам
*/
struct search_node* make_string_attr_cond(int8_t node_order, char* attr_name, enum string_operations operation, char* value);
/**
 * @param schema - очищается при очистке итератора
*/
struct search_node* make_schema_cond(int8_t node_order, struct schema* schema); 
struct search_node* make_and_cond(struct search_node* a, struct search_node* b);
struct search_node* make_or_cond(struct search_node* a, struct search_node* b);
struct search_node* make_not_cond(struct search_node* a);

/**
 * на случай если созданное условие в итоге не понадобилось, а память надо отдать
*/
void free_search_node(struct search_node* cond);

typedef struct Search_Iter {
    struct search_node* cond;
    struct node* node;
    struct file_descriptor* ptr;
    bool is_valid = true;

    Search_Iter(struct file_descriptor* ptr, struct search_node* cond) : ptr(ptr), cond(cond) {
        this->node = read_first_node(ptr);
        this->is_valid = this->next(); // возможно переделать, т.к. сразу должна идти на детей первой ноды
    } 

    bool next();
    struct node* operator*();
    void free();
} Search_Iter;

Search_Iter find_nodes(struct file_descriptor* ptr, struct search_node* cond);

#endif
