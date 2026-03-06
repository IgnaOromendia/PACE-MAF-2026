#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Tree.h"

class Instance {
private:
    std::string name;
    std::string idigest;
    std::string description;

    int treesAmount, labelsAmount;
    
    std::vector<Tree> newickTrees;

    void unquote(std::string& text);
    void trim(std::string& text);

public:
    Instance(std::string fileName);
    ~Instance();
};

#endif