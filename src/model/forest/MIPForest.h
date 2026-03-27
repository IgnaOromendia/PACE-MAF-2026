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

struct TripleHash {
    size_t operator()(const Triple& t) const noexcept {
        size_t seed = std::hash<int>{}(t.i);
        seed ^= std::hash<int>{}(t.j) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(t.k) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct Path {
    int tree, i, j, k, l, score;
    Path(int tree, int i, int j, int k, int l, int score): tree(tree), i(i), j(j), k(k), l(l), score(score) {}

    bool operator==(const Path& other) const {
        return tree == other.tree and i == other.i and j == other.j and
               k == other.k and l == other.l;
    }
};

struct PathHash {
    std::size_t operator()(const Path& p) const noexcept {
        std::size_t seed = std::hash<int>{}(p.tree);
        seed ^= std::hash<int>{}(p.i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(p.j) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(p.k) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(p.l) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class MIPForest: public Forest {
private:
    void addIncompatiblePathPartition(const MIPForest* F, int a, int b, int c, int d, std::unordered_set<Path, PathHash> &incompatible) const;

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
    void conflictiveTriples(const MIPForest* F, std::unordered_set<Triple, TripleHash>& conflictive) const;
    bool isConflictive(const Triple& t, const MIPForest* F) const;
    int amountOfTriples() const;

    // Paths
    void incompatiblePaths(const MIPForest* F, std::unordered_set<Path, PathHash>& incompatible) const;
    
};

#endif
