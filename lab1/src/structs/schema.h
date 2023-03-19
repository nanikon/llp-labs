#ifndef LLP_SCHEMA_H
#define LLP_SCHEMA_H

#include <vector>
#include <stdint.h>
#include <string.h>

#include "../file/file.h"
#include "block.h"

enum value_type {
    INT,
    DOUBLE,
    BOOL,
    STRING
};

#define ATTRIBUTE_SCHEMA_SIZE sizeof(enum value_type) + sizeof(const char*)
struct attribute_schema { 
    enum value_type type;
    const char* name;
};

#define STRUCT_SCHEMA_HEADER_SIZE sizeof(size_t) * 5 // для записи в файл - четыре поля + длина name 
#define SCHEMA_NEXT_OFFSET sizeof(size_t) * 2
#define SCHEMA_COUNT_OFFSET sizeof(size_t) * 3
struct schema {
    size_t offset; // не записывается в файл, вместо него кол-во аттрибутов
    size_t elem_size; // длина блока в файле в котором записана схема - может быть больше фактической длины
    size_t next;
    size_t count; // кол-во вершин с этой схемой - нужно для удаления, точнее для его запрещения
    const char* name; // нуль-терминированная строка
    std::vector<struct attribute_schema*>* attributes;
}; 

/* Как schema хранится в файле:
   - кол-во аттрибутов
   - длина занимаемого ей блока
   - смещение следующей схемы
   - число элементов с этой схемой
   - длина строки с именем схемы
   - имя схемы
   далее список аттрибутов (длина была в начале), состоит из:
    - тип аттрибута
    - длина имени аттрибута
    - имя аттрибута
*/

struct schema* read_first_schema(struct file_descriptor* ptr);
struct schema* read_schema(struct file_descriptor* ptr, size_t offset);
size_t read_schema_next(struct file_descriptor* ptr, size_t schema_offset);
size_t read_schema_count(struct file_descriptor* ptr, size_t schema_offset);

void write_schema(struct file_descriptor* ptr, struct schema* schema);
void update_schema_next(struct file_descriptor* ptr, size_t schema_offset, size_t next);
void update_schema_count(struct file_descriptor* ptr, size_t schema_offset, size_t count);

void free_schema(struct schema* schema);
bool compare_schema(struct schema* first_schema, struct schema* second_schema);
bool check_exist_schema(struct file_descriptor* ptr, struct schema* schema);
// вспомогательные функции, потом убрать из интерфейса
void print_schema(struct schema* schema);


#endif