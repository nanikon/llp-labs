#include "stress_test.h"

char symbols[36] = {
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g',
        'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '1', '2', '3', '4',
        '5', '6', '7', '8', '9', '0'};

int get_random_int(int from, int to) {
    return from + rand() % (to - from + 1);
}

bool get_random_bool() {
    return get_random_int(0, 1) == 1;
}

char* get_random_string(size_t len) {
    char* res = (char*) malloc(sizeof(char) * (len + 1));
    for (int i = 0; i < len; ++i) {
        res[i] = symbols[get_random_int(0, 35)];
    }
    res[len] = 0;
    return res;
}

void test_insert_and_search_node(size_t count) {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");
    std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
    char* attr_name = "just value";
    schema_attr->push_back(create_attribute(STRING, attr_name));
    struct schema* schema = create_schema(ptr, "main schema", schema_attr);

    size_t nodes_counter = 1;    
    size_t* nodes = new size_t[count + 1];
    nodes[0] = 0;

    std::tr1::unordered_map<struct attribute_schema*, union data> attrs;
    struct node* parent = NULL;
    struct node* node;
    int parent_index = 0;
    size_t parent_offset;

    std::vector<double> insert_time(count);
    std::vector<double> search_time(count);

    for (int i = 0; i < count; i++) {
        union data value = { .string_value = get_random_string(get_random_int(1, 10)) };
        attrs.clear();
        attrs[schema_attr->at(0)] = value;

        unsigned int start_time =  clock();
        create_node(ptr, schema, parent, attrs, &node);
        unsigned int end_time = clock();
        unsigned int period = end_time - start_time;
        insert_time[i] = (double) period / CLOCKS_PER_SEC;

        start_time =  clock();
        Search_Iter iter = find_nodes(ptr, make_string_attr_cond(0, attr_name, STRING_EQUAL, (char*) value.string_value));
        while (iter.next());
        end_time = clock();
        iter.free();
        period = end_time - start_time;
        search_time[i] = (double) period / CLOCKS_PER_SEC;

        nodes[nodes_counter] = node->offset;
        nodes_counter++;
        free_node(node);

        int parent_index = get_random_int(0, nodes_counter - 1);
        size_t parent_offset = nodes[parent_index];
        if (parent != NULL) { 
            struct schema* sc = parent->schema;
            free_node(parent); 
            free_schema(sc);
        }
        parent = read_node(ptr, parent_offset);
    }
    // запись в файлы для скрипта, строящего графики
    remove("insert_time");
    remove("search_time");
    FILE* fp1 = fopen("insert_time", "wt");
    FILE* fp2 = fopen("search_time", "wt");
    for (int i = 0; i < count; i++) {
        fprintf(fp1, "%d,%.6f\n", i, insert_time[i]);
        fprintf(fp2, "%d,%f\n", i, search_time[i]);
    }
    fclose(fp1);
    fclose(fp2);

    delete [] nodes;
    if (parent != NULL) { 
        struct schema* sc = parent->schema;
            free_node(parent); 
            free_schema(sc);
    }
    free_schema(schema);
    close_file_db(ptr);
    printf("test insert and search node finished successful\n");
}

void test_update_node(size_t count) {
    remove("test");
    struct file_descriptor* ptr = open_file_db("test");
    std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
    char* attr_name = "just value";
    schema_attr->push_back(create_attribute(STRING, attr_name));
    struct schema* schema = create_schema(ptr, "main schema", schema_attr);

    size_t nodes_counter = 1;    
    size_t* nodes = new size_t[count + 1];
    nodes[0] = 0;

    std::tr1::unordered_map<struct attribute_schema*, union data> attrs;
    struct node* parent = NULL;
    struct node* node;
    int parent_index = 0;
    size_t parent_offset;

    std::vector<double> edit_time(count);

    for (int i = 0; i < count; i++) {
        union data value = { .string_value = get_random_string(get_random_int(1, 10)) };
        attrs.clear();
        attrs[schema_attr->at(0)] = value;
        enum node_create_operation_status status = create_node(ptr, schema, parent, attrs, &node);
        nodes[nodes_counter] = node->offset;
        nodes_counter++;
        free_node(node);

        // само редактирование
            int edit_index = get_random_int(1, nodes_counter - 1);
            size_t edit_offset = nodes[edit_index];
            struct node* edit_node = read_node(ptr, edit_offset);
            union data edit_value = { .string_value = get_random_string(get_random_int(1, 10)) };
            attrs.clear();
            attrs[edit_node->attributes->at(0)->schema] = edit_value;

            unsigned int start_time = clock();
            update_node(ptr, edit_node, attrs);
            unsigned int end_time = clock();
            unsigned int period = end_time - start_time;
            edit_time[i] = (double) period / CLOCKS_PER_SEC;

            if (edit_offset != edit_node->offset) {
                nodes[edit_index] = edit_node->offset;
            }

            struct schema* sc_e = edit_node->schema;
            free_node(edit_node);
            free_schema(sc_e);

        int parent_index = get_random_int(0, nodes_counter - 1);
        size_t parent_offset = nodes[parent_index];
        if (parent != NULL) { 
            struct schema* sc = parent->schema;
            free_node(parent); 
            free_schema(sc);
        }
        parent = read_node(ptr, parent_offset);
    }

    remove("update_time");
    FILE* fp = fopen("update_time", "wt");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%.6f\n", i, edit_time[i]);
    }
    fclose(fp);

    delete [] nodes;
    if (parent != NULL) { 
        struct schema* sc = parent->schema;
            free_node(parent); 
            free_schema(sc);
    }
    free_schema(schema);
    close_file_db(ptr);
    printf("test update node finished successful\n");
}

void test_delete_node(size_t count) {
    std::vector<double> delete_time(count / 10);
    for (int len_tree = 10; len_tree <= count; len_tree += 10) {
        remove("test");
        struct file_descriptor* ptr = open_file_db("test");
        std::vector<struct attribute_schema*>* schema_attr = new std::vector<struct attribute_schema*>;
        char* attr_name = "just value";
        schema_attr->push_back(create_attribute(STRING, attr_name));
        struct schema* schema = create_schema(ptr, "main schema", schema_attr);

        size_t nodes_counter = 1;    
        size_t* nodes = new size_t[len_tree + 1];
        nodes[0] = 0;

        std::tr1::unordered_map<struct attribute_schema*, union data> attrs;
        struct node* parent = NULL;
        struct node* node;
        int parent_index = 0;
        size_t parent_offset;

        for (int i = 0; i < len_tree; i++) {
            union data value = { .string_value = get_random_string(get_random_int(1, 10)) };
            attrs.clear();
            attrs[schema_attr->at(0)] = value;
            enum node_create_operation_status status = create_node(ptr, schema, parent, attrs, &node);
            nodes[nodes_counter] = node->offset;
            nodes_counter++;
            free_node(node);

            int parent_index = get_random_int(0, nodes_counter - 1);
            size_t parent_offset = nodes[parent_index];
            if (parent != NULL) { 
                struct schema* sc = parent->schema;
                free_node(parent); 
                free_schema(sc);
            }
            parent = read_node(ptr, parent_offset);
        }
        /* само удаление */
        int delete_index = get_random_int(1, nodes_counter - 1);
        size_t delete_offset = nodes[delete_index];
        struct node* deleted_node = read_node(ptr, delete_offset);
        unsigned int start_time = clock();
        delete_node(ptr, deleted_node);
        unsigned int end_time = clock();
        unsigned int period = end_time - start_time;
        delete_time[len_tree / 10 - 1] = (double) period / CLOCKS_PER_SEC;

        delete [] nodes;
        if (parent != NULL) { 
            struct schema* sc = parent->schema;
            free_node(parent); 
            free_schema(sc);
        }
        free_schema(schema);
        close_file_db(ptr);
    }

    remove("delete_time");
    FILE* fp = fopen("delete_time", "wt");
    for (int i = 1; i <= delete_time.size(); i++) {
        fprintf(fp, "%d,%.6f\n", i * 10, delete_time[i - 1]);
    }
    fclose(fp);
    printf("test delete node finished successful\n");
}

stress_test_func stress_tests[STRESS_TEST_COUNT] = {
    test_insert_and_search_node,
    test_update_node,
    test_delete_node
};
