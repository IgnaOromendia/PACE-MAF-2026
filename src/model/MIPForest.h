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

public:
    MIPForest(int forestId, int nodeAmount, int labelAmount);
    MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount);
    MIPForest(const MIPForest& other);
    MIPForest(const Forest& other);
    ~MIPForest();

    int modId() const;

    // Edges
    void printEdgeIds() const;

    // Triples
    std::pair<int,int> low(const Triple& t) const;
    void conflictiveTriples(const MIPForest* F, std::vector<Triple>& conflictive) const;
    bool isConflictive(const Triple& t, const MIPForest* F) const;
    int amountOfTriples() const;
    
};

#endif
