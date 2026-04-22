#ifndef MIP_FOREST_H
#define MIP_FOREST_H

#include "Forest.h"

struct ShrinkInfo {
    int parent, sibling, grandparent;
    ShrinkInfo(int p, int s, int g): parent(p), sibling(s), grandparent(g) {}
    ShrinkInfo(): ShrinkInfo(-1,-1,-1) {}
};

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
    int tree, i, j, k, l;
    Path(int tree, int i, int j, int k, int l): tree(tree), i(i), j(j), k(k), l(l){}

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

    const double ALPHA  = 1.0;
    const double BETA   = 1.0;
    const double MU     = 0.01;

    std::vector<std::vector<ShrinkInfo>> nodeContainer;
    
    std::vector<char> originalEdge;

    std::vector<int> amountOfEdgesForTriple;
    std::vector<int> amountOfEdgesForIncomPath;

    std::vector<std::unordered_set<int>> conflictedTriplesForEdge;
    std::vector<std::unordered_set<int>> incompaiblePathPairsForEdge;

    void addIncompatiblePathPartition(const MIPForest* F, int a, int b, int c, int d, std::unordered_set<Path, PathHash> &incompatible);

    void modifyAdjecncy(int a, int b, std::vector<std::pair<int,int>>& adj, std::vector<int>& parent, std::vector<std::vector<ShrinkInfo>>& contains);

public:
    MIPForest(int forestId, int nodeAmount, int amountOfLabels) : Forest(forestId, nodeAmount, amountOfLabels) { originalEdge.assign(nodeAmount-1, 1); };
    MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int amountOfLabels): Forest(forestId, adjacency, parents, amountOfLabels) { originalEdge.assign(adjacency.size()-1, 1); };
    MIPForest(const MIPForest& other);
    MIPForest(const Forest& other);
    ~MIPForest();

    int modId() const;

    // Edges
    void printEdgeIds() const;
    double triplesScore(int e) const;
    double pathsScore(int e) const;
    double edgeScore(int e, MIPForest* F) const;
    double edgeDamage(int e) const;
    bool isOriginal(int e) const;

    // Triples
    std::pair<int,int> low(const Triple& t) const;
    void conflictiveTriples(const MIPForest* F, std::unordered_set<Triple, TripleHash>& conflictive);
    bool isConflictive(const Triple& t, const MIPForest* F) const;
    int amountOfTriples() const;
    int amountOfConflictiveTriplesPerEdge(int e) const;

    // Paths
    void incompatiblePaths(const MIPForest* F, std::unordered_set<Path, PathHash>& incompatible);
    int amountOfIncompatiblePathsPerEdge(int e) const;
    
    // Forest Operations
    void cut(int node);
    void regraft();
    void shrinkWith(MIPForest* F2);
    void expand();
};

#endif
