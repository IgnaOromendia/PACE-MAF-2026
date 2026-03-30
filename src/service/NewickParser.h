#ifndef NEWICKPARSER_H
#define NEWICKPARSER_H

#include <cctype>
#include <string>
#include "../model/forest/Forest.h"

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

    Forest* newickToForest(int id, std::string stringTree, int amountOfLabels);
    void forestToNewick(Forest* forest);
};

#endif
