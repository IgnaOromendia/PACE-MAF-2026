#ifndef MIP_FOREST_H
#define MIP_FOREST_H

#include "Forest.h"

struct Triple {
    Triple(int i, int j, int k): i(i), j(j), k(k) {}
    int i,j,k;
    
    bool operator==(const Triple& other) const {
        return i == other.i and j == other.j and k == other.k;
    } 
};

class MIPForest: public Forest {
private:
    
    // std::unordered_map<std::pair<int,int>, std::vector<int> , EdgeHash> paths;

    void walkAndAdd(int from, int to, std::vector<int>& path) const;
    void precomputPaths();

public:
    MIPForest(int forestId, int nodeAmount, int labelAmount);
    MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    MIPForest(const MIPForest& other);
    MIPForest(const Forest& other);
    ~MIPForest();

    int modId() const;

    // Edges
    std::vector<int> pathBetween(int v, int w) const;
    int pathSize(int v, int w) const;
    void printEdgeIds() const;
    

    // Triples
    std::pair<int,int> low(const Triple& t) const;
    void conflictiveTriples(const MIPForest* F, std::vector<Triple>& conflictive) const;
    bool isConflictive(const Triple& t, const MIPForest* F) const;
    
};

#endif
