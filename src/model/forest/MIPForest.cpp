#include "MIPForest.h"
#include <algorithm>

MIPForest::MIPForest(int forestId, int nodeAmount, int labelAmount) : Forest(forestId, nodeAmount, labelAmount) {}

MIPForest::MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount): Forest(forestId, adjacency, parents, labelAmount) {}

MIPForest::MIPForest(const MIPForest &other): Forest(other) {}

MIPForest::MIPForest(const Forest &other): Forest(other) {}

MIPForest::~MIPForest() {}

int MIPForest::modId() const {
    return id() == 0 ? 0 : 1;
}

void MIPForest::printEdgeIds() const {
    std::vector<std::pair<int, std::pair<int, int>>> edges;
    edges.reserve(nodeToEdge.size());

    for (const auto& [nodes, id] : nodeToEdge)
        edges.push_back({id, nodes});

    std::sort(edges.begin(), edges.end());

    for (const auto& [id, nodes] : edges)
        std::cout << id << ": " << nodes.first << " - " << nodes.second << "\n";
}

std::pair<int,int> MIPForest::low(const Triple& t) const {
    int l_ij = LCA(t.i, t.j);
    int l_ik = LCA(t.i, t.k);
    int l_jk = LCA(t.j, t.k);

    if (l_ij == l_ik) return {t.j, t.k};
    if (l_ij == l_jk) return {t.i, t.k};

    return {t.i, t.j};
}

void MIPForest::conflictiveTriples(const MIPForest* F, std::unordered_set<Triple, TripleHash>& conflictive) const {
    conflictive.clear();
    for(int v = 0; v < labelAmount(); v++) {
        for(int w = v + 1; w < labelAmount(); w++) {
            if (not sameConnectedComponent(v,w)) continue;
            for(int z = w + 1; z < labelAmount(); z++) {
                if (not sameConnectedComponent(w,z)) continue;
                Triple t = Triple(v,w,z);
                if (low(t) != F->low(t)) {
                    auto [j, k] = low(t);
                    int i = v == j or v == k ? (w == j or w == k ? z : w) : v;
                    conflictive.insert(Triple(i, j, k));
                }       
            }
        }
    }       
}

bool MIPForest::isConflictive(const Triple &t, const MIPForest *F) const {
    if (not sameConnectedComponent(t.i, t.j) or not sameConnectedComponent(t.i, t.k) or not sameConnectedComponent(t.j, t.k)) return false;
    return low(t) != F->low(t);
}

int MIPForest::amountOfTriples() const {
    int n = labelsAmount;
    return n * (n-1) * (n-2) / 6;
}

void MIPForest::incompatiblePaths(const MIPForest *F, std::unordered_set<Path, PathHash> &incompatible) const {
    for(int i = 0; i < labelsAmount; i++) {
        for(int j = i+1; j < labelsAmount; j++) {
            if (not sameConnectedComponent(i,j)) continue;
            for(int k = j+1; k < labelsAmount; k++) {
                if (not sameConnectedComponent(i,k)) continue;
                for(int l = k+1; l < labelsAmount; l++) {
                    if (not sameConnectedComponent(k,l)) continue; 

                    addIncompatiblePathPartition(F, i, j, k, l, incompatible);
                    addIncompatiblePathPartition(F, i, k, j, l, incompatible);
                    addIncompatiblePathPartition(F, i, l, j, k, incompatible);
                    

                }
            }
        }
    }
}

void MIPForest::addIncompatiblePathPartition(const MIPForest* F, int a, int b, int c, int d, std::unordered_set<Path, PathHash> &incompatible) const {
    if (not pathIntersection(a,b,c,d) and F->pathIntersection(a,b,c,d)) 
        incompatible.insert(Path(modId(),a, b, c, d, pathScore(a,b) + pathScore(c,d)));
        
    
    if (pathIntersection(a,b,c,d) and not F->pathIntersection(a,b,c,d)) 
        incompatible.insert(Path(F->modId(),a, b, c, d, F->pathScore(a,b) + F->pathScore(c,d)));
}