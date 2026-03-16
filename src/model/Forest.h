#ifndef FOREST_H
#define FOREST_H

#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

class Forest {
protected:
    int forestId, nodeAmount, labelsAmount, treeCount, rootId;

    std::vector<std::pair<int, int> > adj;
    std::vector<int> parent, tree, visited;
    
    void updateComponents(int v);
    bool nodeInRange(int a) const;

public:
    Forest(int forestId, int nodeAmount) : Forest(forestId, nodeAmount, nodeAmount) {};
    Forest(int forestId, int nodeAmount, int labelAmount);
    Forest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    Forest(const Forest& other);
    ~Forest();

    int id() const;

    // Amountos
    int labelAmount() const;
    int amountOfTrees() const;
    int amountOfNodes() const;

    // Nodes
    bool sameConnectedComponent(int a, int b) const;
    int LCA(int a, int b) const;
    bool isLeaf(int a) const;
    
    // Forest Operations
    void cut(int node);
    void regraft();

    // Root
    int root() const;
    int rootChild() const;

    // Debug
    void printAdjAndParents() const;
        
};

#endif
