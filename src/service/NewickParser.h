#ifndef NEWICKPARSER_H
#define NEWICKPARSER_H

#include <cctype>
#include <fstream>
#include <string>
#include "../model/Forest.h"

class NewickParser{
private:
    int index, nodeId, labelsAmount, stringTreeSize, maxNodes;

    std::string stringTree;
    std::vector<std::pair<int,int>> adj;
    std::vector<int> parent;

    Forest* forest = nullptr;

    int parseNodeWithParent(int parentId);
    int getCurrentValue();
    void skipWhitespace();
    std::string serializeNode(int node) const;

public:
    NewickParser();
    ~NewickParser();

    Forest* newickToForest(int id, std::string stringTree, int labelAmount);
    void forestToNewick(Forest* forest, std::string filePath);
};

#endif
