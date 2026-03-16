#ifndef MIP_FOREST_H
#define MIP_FOREST_H

#include "Forest.h"
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <unordered_set>


struct EdgeHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        std::size_t h1 = std::hash<int>{}(p.first);
        std::size_t h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

struct Triple {
    Triple(int i, int j, int k): i(i), j(j), k(k) {}
    int i,j,k;
    
    bool operator==(const Triple& other) const {
        return i == other.i and j == other.j and k == other.k;
    } 
};

struct TripleHash {
    std::size_t operator()(const Triple& t) const {
        std::size_t h1 = std::hash<int>{}(t.i);
        std::size_t h2 = std::hash<int>{}(t.j);
        std::size_t h3 = std::hash<int>{}(t.k);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

class MIPForest: public Forest {
private:

    int edgesAmount;
    
    std::unordered_set<Triple, TripleHash> conflictedLeafs;
    std::vector<std::pair<int, int>> edgeToNode;
    std::unordered_map<std::pair<int,int>, int, EdgeHash> nodeToEdge;
    std::unordered_map<std::pair<int,int>, std::vector<int> , EdgeHash> paths;

    void walkAndAdd(int from, int to, std::vector<int>& path) const;
    void precomputPaths();
    void tagEdges();
    

public:
    MIPForest(int forestId, int nodeAmount, int labelAmount);
    MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    MIPForest(const MIPForest& other);
    MIPForest(const Forest& other);
    ~MIPForest();

    int modId() const;

    // Edges
    int amountOfEdges() const;

    // Edges
    std::vector<int> pathBetween(int v, int w) const;
    int pathSize(int v, int w) const;
    void printEdgeIds() const;
    std::pair<int, int> nodesOf(int edgeId) const;

    // Triples
    std::pair<int,int> low(Triple t) const;
    
    
};

#endif
