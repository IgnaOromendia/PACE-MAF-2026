#ifndef BT_FOREST_H
#define BT_FOREST_H

#include "Forest.h"

struct NodeInfo {
    NodeInfo() = default;
    NodeInfo(int id, int l = -1, int r = -1, bool original = false): original(original), id(id), left(l), right(r) {}
    bool original = true;
    int id = -1, left = -1, right = -1;
};


class BTForest: public Forest {
private:
    std::vector<NodeInfo> nodesInfo;
    BTForest* walkAndPrune(BTForest* f, int from, int to) const;
    void initializeNodeInfo(int amount);

public:
    BTForest(int forestId, int nodeAmount) : BTForest(forestId, nodeAmount, nodeAmount) {};
    BTForest(int forestId, int nodeAmount, int labelAmount);
    BTForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    BTForest(const Forest& other);
    BTForest(const BTForest& other);
    ~BTForest();

    // Siblings info
    bool areSiblings(int a, int b) const;
    std::pair<int,int> siblings() const;

    // Forest Operations
    BTForest* cut(int node) const;
    BTForest* shrink(int a, int b) const;
    BTForest* expand() const;
    BTForest* prunePathBetween(int a, int b) const;
    BTForest* singletons() const;
    
};




#endif