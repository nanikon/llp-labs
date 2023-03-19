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

        struct block first_block = { .next = 0, .prev = 0, .size = DEFAULT_BLOCK_SIZE};
        write_block(&first_block, ptr, FILE_HEADER_SIZE);
        ptr->header->first_free_block = FILE_HEADER_SIZE;
        ptr->header->end_file = get_file_len(fd);

        char* schema_name = (char*) malloc(sizeof(char) * (strlen(schema_name) + 1));
        strcpy(schema_name, "default schema");
        struct schema* schema = create_schema(ptr, schema_name, new std::vector<struct attribute_schema*>); // что-то запишется по адресу ноль, но вроде некритично
        ptr->header->first_schema = schema->offset;
        
        struct node* node = (struct node*) calloc(1, sizeof(struct node));
        node->schema = schema;
        node->attributes = new std::vector<struct attribute*>;
        write_node(ptr, node);
        ptr->header->first_node = node->offset;

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
    ptr = NULL;
}
