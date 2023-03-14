#ifndef LLP_SCHEMA_H
#define LLP_SCHEMA_H

#include <vector>
#include <stdint.h>
#include <string.h>

#include "../file/file.h"

enum type {
    INT,
    DOUBLE,
    BOOL,
    STRING
};

struct attribute_schema { 
    enum type type;
    char* name;
};

#define STRUCT_SHEMA_HEADER_SIZE sizeof(size_t) * 5 // + длина name
struct schema {
    size_t offset; // не записывается в файл, вместо него кол-во аттрибутов
    size_t elem_size; // длина блока в файле в котором записана схема - может быть больше фактической длины
    size_t next;
    size_t count; // кол-во вершин с этой схемой - нужно для удаления, точнее для его запрещения
    const char* name; // нуль-терминированная строка
    std::vector<struct attribute_schema*>* attributes;
}; 

struct schema* read_first_schema(struct file_descriptor* ptr);
struct schema* read_schema(struct file_descriptor* ptr, size_t offset);
size_t read_schema_next(struct file_descriptor* ptr, size_t schema_offset);

void write_schema(struct file_descriptor* ptr, struct schema* schema);
void update_schema_next(struct file_descriptor* ptr, size_t schema_offset, size_t next);

void free_schema(struct schema* schema);
// вспомогательные функции, потом убрать из интерфейса
void print_schema(struct schema* schema);


#endif