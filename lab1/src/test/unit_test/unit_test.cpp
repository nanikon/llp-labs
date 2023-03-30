#include "unit_test.h"

#define BUFFER_LEN 10

static struct attribute* create_attribute(struct attribute_schema* schema, union data value) {
    struct attribute* attr = (struct attribute*) calloc(1, sizeof(struct attribute));
    attr->schema = schema;
    attr->value = value;
    return attr;
}

void test_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);

    int64_t buffer[BUFFER_LEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    size_t result = write_buffer_to_file(fd, 0, buffer, sizeof(int64_t), BUFFER_LEN);
    assert(result != -1);

    int64_t result_buffer[BUFFER_LEN];
    result = read_buffer_from_file(fd, 0, result_buffer, sizeof(int64_t), BUFFER_LEN);
    assert(result != -1);
    for (uint8_t i = 0; i < BUFFER_LEN; i++) {
        assert(buffer[i] == result_buffer[i]);
    }

    close_file(fd);
    printf("test io module finished successful\n");
};

void test_str_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);

    const char* first_str = "Hello world!";
    size_t result = write_null_term_str_to_file(fd, 0, first_str);
    assert(result != -1);

    const char* result_str = NULL;
    result = read_null_term_str_from_file(fd, 0, &(result_str));
    assert(result != -1);
    assert(strcmp(first_str, result_str)== 0);

    close_file(fd);
    printf("test str io module finished successful\n");
}

void test_block_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);
    struct file_descriptor ptr = { .fd = fd, .header = NULL };

    struct block source = { .next = 20, .prev = 30, .size = 100 };
    size_t block_offset = 5;
    write_block(&source, &ptr, block_offset);

    struct block* result = read_block(&ptr, block_offset);

    assert(memcmp(&source, result, sizeof(struct block)) == 0);
    assert(source.prev == read_block_prev(&ptr, block_offset));
    assert(source.size == read_block_size(&ptr, block_offset));

    update_block_prev(&ptr, block_offset, 3);


    free_block(result);
    close_file(fd);
    printf("test block io module finished successful\n");
}

void test_tree_header_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);
    struct tree_header header = { .first_node = 0, .first_schema = 0, .last_schema = 0, .first_free_block = 5*sizeof(size_t), .end_file = 10};
    struct file_descriptor ptr = { .fd = fd, .header = &header };
    write_header(&ptr);
    struct tree_header* result = read_header(fd);
    assert(memcmp(&header, result, sizeof(struct tree_header)) == 0);

    free(result);
    close_file(fd);
    printf("test tree_header io module finished successful\n");
}

void test_schema_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);
    struct tree_header header = { .first_node = 0, .first_schema = 0, .last_schema = 0, .first_free_block = 0, .end_file = 0 };
    struct file_descriptor ptr = { .fd = fd, .header = &header };
    create_block(FILE_HEADER_SIZE, DEFAULT_BLOCK_SIZE, &ptr);
    ptr.header->end_file = get_file_len(fd);

    std::vector<struct attribute_schema*>* attributes = new std::vector<struct attribute_schema*>;
    attributes->push_back(create_attribute(INT, "first_attr"));
    attributes->push_back(create_attribute(DOUBLE, "double attr"));
    attributes->push_back(create_attribute(BOOL, "other attr"));
    attributes->push_back(create_attribute(STRING, "final attr"));
    struct schema* schema = (struct schema*) calloc(1, sizeof(struct schema));
    schema->attributes = attributes;
    copy_str_to_heap(&schema->name, "first schema");
    write_schema(&ptr, schema);

    struct schema* result = read_schema(&ptr, schema->offset);
    assert(compare_schema(schema, result) == true);

    free_schema(result);
    free_schema(schema);
    close_file(fd);
    printf("test schema io module finished successful\n");
}

void test_node_io_module() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);
    struct tree_header header = { .first_node = 0, .first_schema = 0, .last_schema = 0, .first_free_block = 0, .end_file = 0 };
    struct file_descriptor ptr = { .fd = fd, .header = &header };
    create_block(FILE_HEADER_SIZE, DEFAULT_BLOCK_SIZE, &ptr);
    ptr.header->end_file = get_file_len(fd);

    std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
    schema_attr->push_back(create_attribute(INT, "first_attr"));
    schema_attr->push_back(create_attribute(DOUBLE, "double attr"));
    schema_attr->push_back(create_attribute(BOOL, "other attr"));
    schema_attr->push_back(create_attribute(STRING, "final attr"));
    struct schema* schema = (struct schema*) calloc(1, sizeof(struct schema));
    schema->attributes = schema_attr;
    copy_str_to_heap(&schema->name, "first schema");
    write_schema(&ptr, schema);

    std::vector<struct attribute*>* attr = new std::vector<struct attribute*>;
    union data first_value = { .int_value = 12 };
    union data second_value = { .double_value = 0.5 };
    union data third_value = { .bool_value = true };
    union data four_value = {};
    copy_str_to_heap(&four_value.string_value, "value");
    attr->push_back(create_attribute(schema_attr->at(0), first_value));
    attr->push_back(create_attribute(schema_attr->at(1), second_value));
    attr->push_back(create_attribute(schema_attr->at(2), third_value));
    attr->push_back(create_attribute(schema_attr->at(3), four_value));
    struct node* node = (struct node*) calloc(1, sizeof(struct node));
    node->schema = schema;
    node->attributes = attr;
    write_node(&ptr, node);
    assert(node->offset != 0);

    struct node* result = read_node(&ptr, node->offset);
    assert(compare_node(node, result) == true);

    free_node(result);
    free_node(node); //schema закроется здесь
    //free_schema(schema);
    close_file(fd);
    printf("test node io module finished successful\n");    
}

void test_create_delete_block() {
    remove("test");
    int32_t fd = open_file("test");
    assert(fd);
    struct file_descriptor ptr = { .fd = fd, .header = NULL };
    struct block _zero_block = { .next = 0, .prev = 0, .size = 3*sizeof(size_t) };
    struct block* zero_block = &_zero_block;
    size_t zero_block_offset = 5;
    write_block(zero_block , &ptr, zero_block_offset);
    struct tree_header header = { .first_node = 0, .first_schema = 0, .last_schema = 0, .first_free_block = zero_block_offset, .end_file = get_file_len(fd)};
    ptr.header = &header;

    size_t first_block_offset = zero_block_offset + zero_block->size;
    create_block(first_block_offset, 10 * sizeof(size_t), &ptr);
    zero_block = read_block(&ptr, zero_block_offset);
    struct block* first_block = read_block(&ptr, first_block_offset);
    assert(ptr.header->first_free_block == first_block_offset);
    assert(zero_block->prev == first_block_offset);
    assert(first_block->next == zero_block_offset);

    size_t second_block_offset = first_block_offset + first_block->size;
    create_block(second_block_offset, 6*sizeof(size_t), &ptr);
    zero_block = read_block(&ptr, zero_block_offset);
    first_block = read_block(&ptr, first_block_offset);
    struct block* second_block = read_block(&ptr, second_block_offset);
    assert(ptr.header->first_free_block == second_block_offset);
    assert(first_block->prev == second_block_offset);
    assert(second_block->next == first_block_offset);

    delete_block(first_block_offset, &ptr);
    zero_block = read_block(&ptr, zero_block_offset);
    second_block = read_block(&ptr, second_block_offset);
    assert(zero_block->prev == second_block_offset);
    assert(second_block->next == zero_block_offset);

    delete_block(second_block_offset, &ptr);
    zero_block = read_block(&ptr, zero_block_offset);
    assert(ptr.header->first_free_block == zero_block_offset);
    assert(zero_block->prev == 0);

    free_block(zero_block);
    free_block(first_block);
    free_block(second_block);
    close_file(fd);
    printf("test block create|delete module finished successful\n");
}

void test_open_close_file() {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");
    size_t first_free = ptr->header->first_free_block;
    size_t first_schema = ptr->header->first_schema;
    size_t last_schema = ptr->header->last_schema;
    size_t first_node = ptr->header->first_node;
    size_t end_file = ptr->header->end_file;
    close_file_db(ptr);

    ptr = open_file_db("test");
    assert(first_free == ptr->header->first_free_block);
    assert(first_schema == ptr->header->first_schema);
    assert(last_schema == ptr->header->last_schema);
    assert(first_node == ptr->header->first_node);
    assert(end_file == ptr->header->end_file);
    close_file_db(ptr);
    printf("test open|close file module finished successful\n");
}

unit_test_func unit_tests[UNIT_TEST_COUNT] = {
    test_io_module,
    test_str_io_module,
    test_block_io_module,
    test_tree_header_io_module,
    test_schema_io_module,
    test_node_io_module,
    test_create_delete_block,
    test_open_close_file
};
