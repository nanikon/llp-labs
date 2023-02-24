#ifndef LLP_SCHEMA_H
#define LLP_SCHEMA_H

#include <vector>
#include <string>

enum type {
    INT,
    DOUBLE,
    BOOL,
    STRING
};

struct attribute_schema{ 
    std::string name;
    enum type type;
};

struct schema {
    int count;
    std::string name;
    std::vector<struct attribute_schema> attributes;
}; 

#endif