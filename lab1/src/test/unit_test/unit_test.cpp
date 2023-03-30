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

    free_schema(result->schema);
    free_node(result);
    free_schema(node->schema);
    free_node(node);
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

void test_schema_api_module() {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");

    Schema_Iter iter = read_schemas(ptr);
    assert(iter.is_valid == false);
    iter.free();

    std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
    schema_attr->push_back(create_attribute(INT, "first_attr"));
    struct schema* first_schema = create_schema(ptr, "first schema", schema_attr);
    assert(ptr->header->last_schema == first_schema->offset);
    assert(ptr->header->first_schema != first_schema->offset);
    assert(read_schema_next(ptr, ptr->header->first_schema) == first_schema->offset);
    assert(first_schema->prev = ptr->header->first_schema);

    std::vector<struct attribute_schema*>* other_schema_attr = new std::vector<struct attribute_schema*>;
    other_schema_attr->push_back(create_attribute(DOUBLE, "double attr"));
    struct schema* second_schema = create_schema(ptr, "second schema", other_schema_attr);
    assert(ptr->header->last_schema == second_schema->offset);
    assert(ptr->header->first_schema != second_schema->offset);
    first_schema = read_schema(ptr, first_schema->offset); 
    assert(first_schema->next == second_schema->offset);
    assert(second_schema->prev == first_schema->offset);

    iter = read_schemas(ptr);
    assert(iter.is_valid == true);
    assert(compare_schema(iter.operator*(), first_schema) == true);
    assert(iter.next() == true);
    assert(compare_schema(iter.operator*(), second_schema) == true);
    assert(iter.next() == false);
    iter.free();

    assert(delete_schema(ptr, first_schema) == OK_SCHEMA_DELETE);
    second_schema = read_schema(ptr, second_schema->offset);
    assert(read_schema_next(ptr, ptr->header->first_schema) == second_schema->offset);
    assert(second_schema->prev == ptr->header->first_schema);

    iter = read_schemas(ptr);
    assert(iter.is_valid == true);
    assert(compare_schema(iter.operator*(), second_schema) == true);
    assert(iter.next() == false);
    iter.free();

    std::vector<struct attribute_schema*>* third_schema_attr = new std::vector<struct attribute_schema*>;
    other_schema_attr->push_back(create_attribute(BOOL, "ba"));
    struct schema* third_schema = create_schema(ptr, "ts", third_schema_attr);
    assert(ptr->header->last_schema == third_schema->offset);
    assert(third_schema->offset < second_schema->offset);

    assert(delete_schema(ptr, third_schema) ==  OK_SCHEMA_DELETE);
    second_schema = read_schema(ptr, second_schema->offset);
    assert(ptr->header->last_schema == second_schema->offset);
    assert(second_schema->next == 0);

    free_schema(second_schema);
    close_file_db(ptr);
    printf("test create|delete|search schema module finished successful\n");
}

void test_node_manupulate_api_module() {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");
    // сделать схему. Кажется лучше с значением строки, чтобы размер и местоположение менять
    std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
    schema_attr->push_back(create_attribute(STRING, "altavista"));
    struct schema* schema = create_schema(ptr, "main schema", schema_attr);
    // создать две ноды у корня
    std::tr1::unordered_map<struct attribute_schema*, union data> attrs;
    struct node* first_node;
    union data first_value;
    copy_str_to_heap(&first_value.string_value, "first node");
    attrs[schema_attr->at(0)] = first_value;
    assert(create_node(ptr, schema, NULL, attrs, &first_node) == OK_NODE_CREATE);
    assert(compare_node(first_node, read_node(ptr, first_node->offset)) == true);
    assert(first_node->parent == ptr->header->first_node);

    struct node* second_node;
    union data second_value;
    copy_str_to_heap(&second_value.string_value, "second node");
    attrs[schema_attr->at(0)] = second_value;
    assert(create_node(ptr, schema, NULL, attrs, &second_node) == OK_NODE_CREATE);
    assert(second_node->next_sibiling == first_node->offset);
    first_node = read_node(ptr, first_node->offset);
    assert(first_node->prev_sibiling == second_node->offset);
    // у первой создать три ребенка
    struct node* first_child;
    union data first_child_value;
    copy_str_to_heap(&first_child_value.string_value, "first_child node");
    attrs.clear();
    attrs[schema_attr->at(0)] = first_child_value;
    assert(create_node(ptr, schema, second_node, attrs, &first_child) == OK_NODE_CREATE);

    struct node* second_child;
    union data second_child_value;
    copy_str_to_heap(&second_child_value.string_value, "second_child node");
    attrs.clear();
    attrs[schema_attr->at(0)] = second_child_value;
    assert(create_node(ptr, schema, second_node, attrs, &second_child) == OK_NODE_CREATE);

    struct node* third_child;
    union data third_child_value;
    copy_str_to_heap(&third_child_value.string_value, "third_child node");
    attrs[schema_attr->at(0)] = third_child_value;
    assert(create_node(ptr, schema, second_node, attrs, &third_child) == OK_NODE_CREATE);
    // изменить первую (с переносом места), проверить что дети/брат/родитель правильно на неё ссылаются
    size_t first_offset_second_node = second_node->offset;
    union data new_value;
    copy_str_to_heap(&new_value.string_value, "second node modifyed");
    attrs.clear();
    attrs[second_node->schema->attributes->at(0)] = new_value;
    assert(update_node(ptr, second_node, attrs) == OK_NODE_UPDATE);
    assert(first_offset_second_node != second_node->offset);
    assert(read_first_child_offset(ptr, ptr->header->first_node) == second_node->offset);
    assert(read_parent_offset(ptr, first_child->offset) == second_node->offset);
    assert(read_parent_offset(ptr, second_child->offset) == second_node->offset);
    assert(read_parent_offset(ptr, third_child->offset) == second_node->offset);
    assert(read_prev_sibiling_offset(ptr, first_node->offset) == second_node->offset);
    // удалить среднего ребенка, проверить что браться правильно ссылаются
    assert(delete_node(ptr, second_child) == OK_NODE_DELETE);
    assert(read_prev_sibiling_offset(ptr, first_child->offset) == third_child->offset);
    assert(read_next_sibiling_offset(ptr, third_child->offset) == first_child->offset); 
    // удалеить третьего ребенка, проверить что родитель сменил ссылку
    assert(delete_node(ptr, third_child) == OK_NODE_DELETE);
    assert(read_first_child_offset(ptr, second_node->offset) == first_child->offset);
    // удалить первую ноду, как-то проверить, что удалился и первый ребенок 
    assert(delete_node(ptr, second_node) == OK_NODE_DELETE);
    assert(read_block_size(ptr, first_child->offset) == first_child->elem_size); // можно т.к. указатель на first_child не очищался

    free_node(first_node);
    free_node(first_child);
    free_schema(schema);
    close_file_db(ptr);
    printf("test create|update|delete node module finished successful\n");
}

void test_node_search_api_module() {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");

    close_file_db(ptr);
    printf("test search node module finished successful\n");
}

unit_test_func unit_tests[UNIT_TEST_COUNT] = {
    test_io_module,
    test_str_io_module,
    test_block_io_module,
    test_tree_header_io_module,
    test_schema_io_module,
    test_node_io_module,
    test_create_delete_block,
    test_open_close_file,
    test_schema_api_module,
    test_node_manupulate_api_module,
    test_node_search_api_module
};
