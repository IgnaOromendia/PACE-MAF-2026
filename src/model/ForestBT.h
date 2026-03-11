#ifndef FORESTBT_H
#define FORESTBT_H

#include "Forest.h"

struct NodeInfo {
    NodeInfo() = default;
    NodeInfo(bool isOriginal, int l, int r): original(isOriginal), left(l), right(r) {}
    bool original = true;
    int left = -1;
    int right = -1;
};


class ForestBT: Forest {
private:
    std::vector<NodeInfo> nodesInfo;
    ForestBT* walkAndPrune(ForestBT* f, int from, int to) const;

public:
    ForestBT(int nodeAmount) : ForestBT(nodeAmount, nodeAmount) {};
    ForestBT(int nodeAmount, int labelAmount);
    ForestBT(std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    ForestBT(const ForestBT& other);
    ~ForestBT();

    // Siblings info
    bool areSiblings(int a, int b) const;
    std::pair<int,int> siblings() const;

    // Forest Operations
    ForestBT* cut(int node) const;
    ForestBT* shrink(int a, int b) const;
    ForestBT* expand();
    ForestBT* prunePathBetween(int a, int b) const;
    
};




#endif