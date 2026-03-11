#ifndef FOREST_H
#define FOREST_H

#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

struct NodeInfo {
    NodeInfo() = default;
    NodeInfo(bool isOriginal, int l, int r): original(isOriginal), left(l), right(r) {}
    bool original = true;
    int left = -1;
    int right = -1;
};


class Forest {
private:
    int nodeAmount, labelsAmount, treeCount, rootId;

    std::vector<std::pair<int, int> > adj;
    std::vector<int> parent, tree, visited;
    std::vector<NodeInfo> nodesInfo;

    void updateComponents(int v);
    Forest* walkAndPrune(Forest* f, int from, int to) const;
    bool nodeInRange(int a) const;

public:
    Forest(int nodeAmount) : Forest(nodeAmount, nodeAmount) {};
    Forest(int nodeAmount, int labelAmount);
    Forest(std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    Forest(const Forest& other);
    ~Forest();

    // Forest info
    int labelAmount() const;
    bool sameConnectedComponent(int a, int b) const;
    int amountOfTrees() const;
    int LCA(int a, int b) const;
    int root() const;
    int rootChild() const;
    void printAdjAndParents() const;

    // Siblings info
    bool areSiblings(int a, int b) const;
    std::pair<int,int> siblings() const;

    // Forest Operations
    Forest* cut(int node) const;
    Forest* shrink(int a, int b) const;
    Forest* prunePathBetween(int a, int b) const;
    Forest* expand();
};

#endif
