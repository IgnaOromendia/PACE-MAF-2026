#ifndef FOREST_H
#define FOREST_H

#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

class Forest {
protected:
    int nodeAmount, labelsAmount, treeCount, rootId;

    std::vector<std::pair<int, int> > adj;
    std::vector<int> parent, tree, visited;
    
    void updateComponents(int v);
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
        
};

#endif
