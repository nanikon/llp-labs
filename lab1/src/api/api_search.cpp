#include "api.h"

bool check_schema(struct node_condition* node_c, struct node* candidat) {
    return compare_schema(node_c->schema_c->schema, candidat->schema);
}

bool check_int(struct int_condition* cond, struct attribute* attr) {
    switch (cond->operation) {
        case DIGIT_EQUAL:
            return attr->value.int_value == cond->value;
        case DIGIT_NOT_EQUAL:
            return attr->value.int_value != cond->value;
        case DIGIT_LESS:
            return attr->value.int_value < cond->value;
        case DIGIT_LESS_OR_EQUAL:
            return attr->value.int_value <= cond->value;
        case DIGIT_GREAT_OR_EQUAL:
            return attr->value.int_value >= cond->value;
        case DIGIT_GREAT:
            return attr->value.int_value > cond->value;
    }
    return false;
}

bool check_double(struct double_condition* cond, struct attribute* attr) {
    switch (cond->operation) {
        case DIGIT_EQUAL:
            return attr->value.double_value == cond->value;
        case DIGIT_NOT_EQUAL:
            return attr->value.double_value != cond->value;
        case DIGIT_LESS:
            return attr->value.double_value < cond->value;
        case DIGIT_LESS_OR_EQUAL:
            return attr->value.double_value <= cond->value;
        case DIGIT_GREAT_OR_EQUAL:
            return attr->value.double_value >= cond->value;
        case DIGIT_GREAT:
            return attr->value.double_value > cond->value;
    }
    return false;
}

bool check_bool(struct bool_condition* cond, struct attribute* attr) {
    switch(cond->operation) {
        case BOOL_EQUAL:
            return attr->value.bool_value == cond->value;
        case BOOL_NOT_EQUAL:
            return attr->value.bool_value != cond->value;
    }
    return false;
}

bool check_string(struct string_condition* cond, struct attribute* attr) {
    switch(cond->operation) {
        case STRING_EQUAL:
            return strcmp(attr->value.string_value, cond->value) == 0;
        case STRING_NOT_EQUAL:
            return strcmp(attr->value.string_value, cond->value) != 0;
        case STRING_CONTAINS:
            return strstr(attr->value.string_value, cond->value) != NULL;
    }
    return false;
}

bool check_attr(struct node_condition* node_c, struct node* candidat) {
    for (int i = 0; i < candidat->attributes->size(); i++) {
        if (node_c->attr_c->type == candidat->attributes->at(i)->schema->type && strcmp(node_c->attr_c->name, candidat->attributes->at(i)->schema->name) == 0) {
            switch (node_c->attr_c->type) {
                case INT:
                    return check_int(node_c->attr_c->int_c, candidat->attributes->at(i));
                case DOUBLE:
                    return check_double(node_c->attr_c->double_c, candidat->attributes->at(i));
                case BOOL:
                    return check_bool(node_c->attr_c->bool_c, candidat->attributes->at(i));
                case STRING:
                    return check_string(node_c->attr_c->string_c, candidat->attributes->at(i));
            }
        }
    }
    return false;
}

bool compute_node_cond(struct file_descriptor* ptr, bool (*cond)(struct node_condition* node_c, struct node* candidat), struct node_condition* node_c, struct node* candidat) {
    if (node_c->node_order == 0) {
        return cond(node_c, candidat);
    } else if (node_c->node_order < 0) {
        // просто поднятся до родителя нужного уровня, при этом проверить чтобы в голову не уперлись? тогда просто false вернем
        size_t parent_offset = candidat->parent;
        if (!check_not_root(ptr, parent_offset)) {
                return false;
            }
        for (int i = 0; i < (node_c->node_order * -1) - 1; i++) {
            parent_offset = read_parent_offset(ptr, parent_offset);
            if (!check_not_root(ptr, parent_offset)) {
                return false;
            }
        }
        struct node* parent = read_node(ptr, parent_offset);
        bool result = cond(node_c, parent);
        free_node(parent);
        return result;
    } else {
        // как-то пройтись по всем детям определенного уровня
        size_t current_order = 1;
        size_t child_offset = candidat->first_child;
        bool result = false;
        do {
            if (current_order == node_c->node_order) {
                struct node* current_child = read_node(ptr, child_offset);
                result = cond(node_c, current_child);
                if (result) { // если вершина подошла то чистим её и прерываемся
                    free_node(current_child);
                    break; 
                }
                if (current_child->next_sibiling != 0) { // если не подошла и есть следующий брат - проверяем его
                    child_offset = current_child->next_sibiling;
                } else { // если следующего брата нет - возвращаемся вверх по дереву пока он не появится 
                    current_order--;
                    child_offset = current_child->parent;
                    free_node(current_child);
                    size_t new_child_offset = read_next_sibiling_offset(ptr, child_offset);
                    while (new_child_offset == 0) { // поднимаемся до тех пор пока не встречаем следующего брата
                        child_offset = read_parent_offset(ptr, child_offset);
                        current_order--;
                        if (current_order == 0) { break; } // или пока не приходим в исходную вершину
                        new_child_offset = read_next_sibiling_offset(ptr, child_offset);
                    }
                    child_offset = new_child_offset;
                }
            } else {
                size_t new_child_offset = read_first_child_offset(ptr, child_offset);
                if (new_child_offset != 0) {
                    child_offset = new_child_offset;
                    current_order++;
                } else {
                    new_child_offset = read_next_sibiling_offset(ptr, child_offset); // идем на следующего брата
                    while (new_child_offset == 0) { // поднимаемся до тех пор пока не встречаем следующего брата
                        child_offset = read_parent_offset(ptr, child_offset);
                        current_order--;
                        if (current_order == 0) { break; } // или пока не приходим в исходную вершину
                        new_child_offset = read_next_sibiling_offset(ptr, child_offset);
                    }
                    child_offset = new_child_offset;
                }
            }
        } while (current_order > 0);
        return result;
    }
}

bool compute_search(struct file_descriptor* ptr, struct search_node* cond, struct node* candidat) {
    switch (cond->type) {
        case OR:
            return compute_search(ptr, cond->or_c->a, candidat) || compute_search(ptr, cond->or_c->b, candidat);
        case AND:
            return compute_search(ptr, cond->or_c->a, candidat) && compute_search(ptr, cond->or_c->b, candidat);
        case NOT:
            return !compute_search(ptr, cond->or_c->a, candidat);
        case NODE:
            switch (cond->node_c->type) {
                case SCHEMA:
                    return compute_node_cond(ptr, check_schema, cond->node_c, candidat);
                case ATTR:
                    return compute_node_cond(ptr, check_attr, cond->node_c, candidat);
            }
    }
    return false;
}

Search_Iter find_nodes(struct file_descriptor* ptr, struct search_node* cond) {
    Search_Iter res = Search_Iter(ptr, cond);
    return res;
}

struct node* Search_Iter::operator*() {
    return node;
}

bool Search_Iter::next() {
    if (!is_valid) {
        return false;
    }

    do {
        if (node->first_child != 0) {
            replace_node(ptr, node->first_child, &node);
        } else if (node->next_sibiling != 0) {
            replace_node(ptr, node->next_sibiling, &node);
        } else {
            while (node->next_sibiling == 0) {
                if (!check_not_root(ptr, node->offset)) {
                    is_valid = false;
                    return false;
                }
                replace_node(ptr, node->parent, &node);
            }

            //мы либо вышли из итератора с false, либо у нас появился следующий сибилинг и можем на него поменятся
            replace_node(ptr, node->next_sibiling, &node);
        }
    } while (!compute_search(ptr, cond, node));

    return true;
}

void Search_Iter::free() {
    if (node != NULL) {
        free_node(node);
    }
    free_search_node(cond);
}

struct search_node* make_int_attr_cond(int8_t node_order, char* attr_name, enum digit_operations operation, int value) {
    struct int_condition* int_cond = (struct int_condition*) malloc(sizeof(struct int_condition));
    int_cond->value = value;
    int_cond->operation = operation;

    struct attr_condition* attr_cond = (struct attr_condition*) malloc(sizeof(struct attr_condition));
    attr_cond->type = INT;
    copy_str_to_heap((const char**) &attr_cond->name, attr_name);
    attr_cond->int_c = int_cond;

    struct node_condition* node_cond = (struct node_condition*) malloc(sizeof(struct node_condition));
    node_cond->type = ATTR;
    node_cond->node_order = node_order;
    node_cond->attr_c = attr_cond;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NODE;
    result->node_c = node_cond;
    return result;
}

struct search_node* make_double_attr_cond(int8_t node_order, char* attr_name, enum digit_operations operation, double value) {
    struct double_condition* double_cond = (struct double_condition*) malloc(sizeof(struct double_condition));
    double_cond->value = value;
    double_cond->operation = operation;

    struct attr_condition* attr_cond = (struct attr_condition*) malloc(sizeof(struct attr_condition));
    attr_cond->type = DOUBLE;
    copy_str_to_heap((const char**) &attr_cond->name, attr_name);
    attr_cond->double_c = double_cond;

    struct node_condition* node_cond = (struct node_condition*) malloc(sizeof(struct node_condition));
    node_cond->type = ATTR;
    node_cond->node_order = node_order;
    node_cond->attr_c = attr_cond;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NODE;
    result->node_c = node_cond;
    return result;
}

struct search_node* make_bool_attr_cond(int8_t node_order, char* attr_name, enum bool_operations operation, bool value) {
    struct bool_condition* bool_cond = (struct bool_condition*) malloc(sizeof(struct bool_condition));
    bool_cond->value = value;
    bool_cond->operation = operation;

    struct attr_condition* attr_cond = (struct attr_condition*) malloc(sizeof(struct attr_condition));
    attr_cond->type = BOOL;
    copy_str_to_heap((const char**) &attr_cond->name, attr_name);
    attr_cond->bool_c = bool_cond;

    struct node_condition* node_cond = (struct node_condition*) malloc(sizeof(struct node_condition));
    node_cond->type = ATTR;
    node_cond->node_order = node_order;
    node_cond->attr_c = attr_cond;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NODE;
    result->node_c = node_cond;
    return result;
}

struct search_node* make_string_attr_cond(int8_t node_order, char* attr_name, enum string_operations operation, char* value) {
    struct string_condition* string_cond = (struct string_condition*) malloc(sizeof(struct string_condition));
    copy_str_to_heap((const char**) &string_cond->value, value);
    string_cond->operation = operation;

    struct attr_condition* attr_cond = (struct attr_condition*) malloc(sizeof(struct attr_condition));
    attr_cond->type = STRING;
    copy_str_to_heap((const char**) &attr_cond->name, attr_name);
    attr_cond->string_c = string_cond;

    struct node_condition* node_cond = (struct node_condition*) malloc(sizeof(struct node_condition));
    node_cond->type = ATTR;
    node_cond->node_order = node_order;
    node_cond->attr_c = attr_cond;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NODE;
    result->node_c = node_cond;
    return result;
}

struct search_node* make_schema_cond(int8_t node_order, struct schema* schema) {
    struct schema_condition* schema_cond = (struct schema_condition*) malloc(sizeof(struct schema_condition));
    schema_cond->schema = schema;

    struct node_condition* node_cond = (struct node_condition*) malloc(sizeof(struct node_condition));
    node_cond->type = SCHEMA;
    node_cond->node_order = node_order;
    node_cond->schema_c = schema_cond;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NODE;
    result->node_c = node_cond;
    return result;
}

struct search_node* make_and_cond(struct search_node* a, struct search_node* b) {
    struct and_condition* and_cond = (struct and_condition*) malloc(sizeof(struct and_condition));
    and_cond->a = a;
    and_cond->b = b;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = AND;
    result->and_c = and_cond;
    return result;
}

struct search_node* make_or_cond(struct search_node* a, struct search_node* b) {
    struct or_condition* or_cond = (struct or_condition*) malloc(sizeof(struct or_condition));
    or_cond->a = a;
    or_cond->b = b;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = OR;
    result->or_c = or_cond;
    return result;
}

struct search_node* make_not_cond(struct search_node* a) {
    struct not_condition* not_cond = (struct not_condition*) malloc(sizeof(struct not_condition));
    not_cond->a = a;

    struct search_node* result = (struct search_node*) malloc(sizeof(struct search_node));
    result->type = NOT;
    result->not_c = not_cond;
    return result;
}

void free_search_node(struct search_node* cond) {
    switch (cond->type) {
        case AND:
            free_search_node(cond->and_c->a);
            free_search_node(cond->and_c->b);
            free(cond->and_c);
            break;
        case OR:
            free_search_node(cond->or_c->a);
            free_search_node(cond->or_c->b);
            free(cond->or_c);
            break;
        case NOT:
            free_search_node(cond->not_c->a);
            free(cond->not_c);
            break;
        case NODE:
            switch(cond->node_c->type) {
                case ATTR:
                    switch(cond->node_c->attr_c->type) {
                        case INT:
                            free(cond->node_c->attr_c->int_c);
                            break;
                        case DOUBLE:
                            free(cond->node_c->attr_c->double_c);
                            break;
                        case BOOL:
                            free(cond->node_c->attr_c->bool_c);
                            break;
                        case STRING:
                            free(cond->node_c->attr_c->string_c->value);
                            free(cond->node_c->attr_c->string_c);
                            break;
                    };
                    free(cond->node_c->attr_c);
                    break;
                case SCHEMA:
                    free_schema(cond->node_c->schema_c->schema);
                    free(cond->node_c->schema_c);
            };
            free(cond->node_c);
            break;
    };
    free(cond);
}
