#ifndef FOREST_H
#define FOREST_H

#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <unordered_set>


struct EdgeHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        std::size_t h1 = std::hash<int>{}(p.first);
        std::size_t h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

class Forest {
protected:
    int edgesAmount, forestId, nodeAmount, labelsAmount, treeCount, rootId;

    std::vector<std::pair<int, int> > adj;
    std::vector<int> parent, tree, visited;
    std::vector<bool> edgeAvailable;
    std::vector<std::pair<int, int>> edgeToNode;
    std::unordered_map<std::pair<int,int>, int, EdgeHash> nodeToEdge;
    std::unordered_map<std::pair<int,int>, std::vector<int> , EdgeHash> paths;
    std::vector<std::unordered_set<std::pair<int, int>, EdgeHash>> leafsForEdge;
    
    void updateComponents(int v);
    bool nodeInRange(int a) const;
    void tagEdges();
    void walkAndAdd(int from, int lca, int to, std::vector<int>& path);
    void precomputeLeafPaths();
    void precomputeAllPaths();
    void precomputePaths(int limit);
    void updatePathsRemoving(int descendantEdge);

public:
    Forest(int forestId, int nodeAmount) : Forest(forestId, nodeAmount, nodeAmount) {};
    Forest(int forestId, int nodeAmount, int labelAmount);
    Forest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    Forest(const Forest& other);
    ~Forest();

    int id() const;

    // Amounts
    int labelAmount() const;
    int amountOfTrees() const;
    int amountOfNodes() const;

    // Nodes
    bool sameConnectedComponent(int a, int b) const;
    int LCA(int a, int b) const;
    bool isLeaf(int a) const;
    std::pair<int, int> childrenOf(int node) const;
    int parentOf(int node) const;
    void removeNodeFromAdj(int node);
    int sibling(int node) const;

    // Edges
    int amountOfEdges() const;
    std::pair<int, int> nodesOf(int edgeId) const;
    bool edgeIsAvailable(int edgeId) const;
    void removeEdge(int v, int u);
    std::vector<int> pathBetween(int v, int w) const;
    int pathSize(int v, int w) const;
    
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
