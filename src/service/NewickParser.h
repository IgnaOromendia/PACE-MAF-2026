#ifndef NEWICKPARSER_H
#define NEWICKPARSER_H

#include <cctype>
#include <string>
#include "../model/Forest.h"

class NewickParser{
private:
    int index = 0;
    int nodeId, labelsAmount, stringTreeSize, maxNodes;

    std::string stringTree;
    std::vector<std::pair<int,int>> adj;
    std::vector<int> parent;

    int parseNodeWithParent(int parentId);
    int getCurrentValue();
    void skipWhitespace();

public:
    NewickParser(std::string stringTree, int labels);
    ~NewickParser();

    Forest* parse(int id);
};

#endif
