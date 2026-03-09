#ifndef FOREST_H
#define FOREST_H

#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

struct LabelInfo {
    LabelInfo() = default;
    LabelInfo(bool isOriginal, int l, int r): original(isOriginal), left(l), right(r) {}
    bool original = true;
    int left = -1;
    int right = -1;
};


class Forest {
private:
    int nodes, labelsAmount, treeCount;

    std::vector<std::pair<int, int> > adj;
    std::vector<int> parent, tree, visited;
    std::vector<LabelInfo> labelInfo;

    void updateComponents(int v);
    void walkAndPrune(Forest* f, int from, int to) const;

public:
    Forest(int nodeAmount) : Forest(nodeAmount, nodeAmount) {};
    Forest(int nodeAmount, int labelAmount);
    Forest(const std::vector<std::pair<int, int>>& adjacency,
           const std::vector<int>& parents,
           int labelAmount);
    Forest(const Forest& other);
    ~Forest();

    // Forest info
    int labelAmount() const;
    bool sameConnectedComponent(int a, int b) const;
    int amountOfTrees() const;
    int LCA(int a, int b) const;
    int rootChild() const;
    void printAdjAndParents() const;

    // Siblings info
    bool areSiblings(int a, int b) const;
    std::pair<int,int> siblings() const;

    // Forest Operations
    Forest* cut(int node) const;
    Forest* shrink(int a, int b) const;
    Forest* prunePathBetween(int a, int b) const;
};

#endif
