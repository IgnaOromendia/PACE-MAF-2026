#ifndef NEWICKPARSER_H
#define NEWICKPARSER_H

#include <string>
#include "Tree.h"

class NewickParser{
private:
    int index;
    int nodesAmount;
    std::string stringTree;

    Node* parseNode();

public:
    NewickParser();
    ~NewickParser();

    Tree parse(std::string stringTree, int labels);
};

#endif
