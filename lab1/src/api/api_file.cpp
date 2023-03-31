#include "api.h"

struct file_descriptor* open_file_db(const char* filename) {
    struct file_descriptor* ptr = (struct file_descriptor*) malloc(sizeof(struct file_descriptor));
    int32_t fd = open_file(filename);
    ptr->fd = fd;
    size_t size = get_file_len(fd);
    if (size <= 1) {
        // файл новый, только создался, логика по заполнению хэдера и потом его запись
        // логика - создать нулевую схему, и нулевую вершину с ней. Записать это в файл
        ptr->header = (struct tree_header*) calloc(1, sizeof(struct tree_header));

        create_block(FILE_HEADER_SIZE, DEFAULT_BLOCK_SIZE, ptr);
        ptr->header->end_file = get_file_len(fd);

        struct schema* schema = (struct schema*) calloc(1, sizeof(struct schema));
        schema->attributes = new std::vector<struct attribute_schema*>;
        copy_str_to_heap(&schema->name, "default schema");
        write_schema(ptr, schema);
        ptr->header->first_schema = schema->offset;
        ptr->header->last_schema = schema->offset;
        
        struct node* node = (struct node*) calloc(1, sizeof(struct node));
        node->schema = schema;
        node->attributes = new std::vector<struct attribute*>;
        write_node(ptr, node);
        ptr->header->first_node = node->offset;
        ptr->header->end_file = get_file_len(fd);

        write_header(ptr);
        free_node(node);
        free_schema(schema);
    } else {
        // файл уже существовал, надо только вычитать хедер
        ptr->header = read_header(fd);
    }
    return ptr;
}

void close_file_db(struct file_descriptor* ptr) {
    write_header(ptr);
    close_file(ptr->fd);
    free(ptr->header);
    free(ptr);
}
