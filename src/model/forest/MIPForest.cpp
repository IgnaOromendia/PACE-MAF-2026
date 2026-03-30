#include "MIPForest.h"
#include <algorithm>
#include <cassert>

MIPForest::MIPForest(int forestId, int nodeAmount, int amountOfLabels) : Forest(forestId, nodeAmount, amountOfLabels) {
    conflictedTriplesForEdge.assign(edgesAmount, std::vector<int>());
    incompaiblePathPairsForEdge.assign(edgesAmount, std::vector<int>());
}

MIPForest::MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int amountOfLabels): Forest(forestId, adjacency, parents, amountOfLabels) {
    conflictedTriplesForEdge.assign(edgesAmount, std::vector<int>());
    incompaiblePathPairsForEdge.assign(edgesAmount, std::vector<int>());
}

MIPForest::MIPForest(const MIPForest &other): Forest(other) {
    conflictedTriplesForEdge = other.conflictedTriplesForEdge;
    tripleSize = other.tripleSize;
    incompaiblePathPairsForEdge = other.incompaiblePathPairsForEdge;
    incomPathSize = other.incomPathSize;
}

MIPForest::MIPForest(const Forest &other): Forest(other) {
    conflictedTriplesForEdge.assign(edgesAmount, std::vector<int>());
    incompaiblePathPairsForEdge.assign(edgesAmount, std::vector<int>());
}

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

double MIPForest::edgeScore(int e) const {
    double score = 0;

    if (not edgeAvailable[e]) return 0;

    for (int t: conflictedTriplesForEdge[e]) {
        int size = tripleSize[t];
        score += 1.0 / (size * size); // Least restrictions involved better
    }

    for (int p: incompaiblePathPairsForEdge[e]) {
        int size = incomPathSize[p];
        score += 1.0 / (size * size);
    }

    return score;
}

std::pair<int,int> MIPForest::low(const Triple& t) const {
    int l_ij = LCA(t.i, t.j);
    int l_ik = LCA(t.i, t.k);
    int l_jk = LCA(t.j, t.k);

    if (l_ij == l_ik) return {t.j, t.k};
    if (l_ij == l_jk) return {t.i, t.k};

    return {t.i, t.j};
}

void MIPForest::conflictiveTriples(const MIPForest* F, std::unordered_set<Triple, TripleHash>& conflictive) {
    conflictive.clear();
    conflictedTriplesForEdge.assign(edgesAmount, std::vector<int>());
    tripleSize.clear();

    for(int v = 0; v < amountOfLabels(); v++) {
        for(int w = v + 1; w < amountOfLabels(); w++) {
            if (not sameConnectedComponent(v,w)) continue;
            for(int z = w + 1; z < amountOfLabels(); z++) {
                if (not sameConnectedComponent(w,z)) continue;
                Triple t = Triple(v,w,z);
                if (low(t) != F->low(t)) {
                    auto [j, k] = low(t);
                    int i = v == j or v == k ? (w == j or w == k ? z : w) : v;

                    int id = conflictive.size();
                    const int triplePathSize = pathSize(i,j) + pathSize(i,k);

                    conflictive.insert(Triple(i, j, k));
                    tripleSize.push_back(triplePathSize);

                    for (int e: pathBetween(i,j))
                        conflictedTriplesForEdge[e].push_back(id);
                    
                    for (int e: pathBetween(i,k))
                        conflictedTriplesForEdge[e].push_back(id);

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

void MIPForest::incompatiblePaths(const MIPForest *F, std::unordered_set<Path, PathHash> &incompatible) {
    incompatible.clear();
    incompaiblePathPairsForEdge.assign(edgesAmount, std::vector<int>());
    incomPathSize.clear();

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

void MIPForest::cut(int node) {
    int p = parent[node];

    if (p != -1) {
        if (adj[p].first == node) adj[p] = {-1, adj[p].second};
        if (adj[p].second == node) adj[p] = {adj[p].first, -1};

        int edgeId = nodeToEdge.at({node, p});
        edgeAvailable[edgeId] = false;
    }

    parent[node] = -1;

    tree[node] = treeCount++;

    visited.assign(nodeAmount, 0);

    timer = 0;
    updateComponents(node);
}

void MIPForest::regraft() {
    bool modified = true;
    
    while(modified) {
        modified = false;
        for(int node = labelsAmount; node < nodeAmount; node++) {
            if (adj[node].first != -1 and adj[node].second != -1) continue;
            if (adj[node].first == -1 and adj[node].second == -1 and parent[node] == -1) continue;

            modified = true;

            int descendant  = adj[node].first == -1 ? adj[node].second : adj[node].first;
            int ancestor    = parent[node];

            if (descendant != -1 and ancestor != -1) {
                int descendantEdge = nodeToEdge.at({descendant, node});
                int newEdgeId = nodeToEdge.at({node, ancestor});;

                removeEdge(descendant, node);
                removeEdge(node, ancestor);

                nodeToEdge.insert({{descendant, ancestor}, newEdgeId});
                edgeAvailable[newEdgeId] = true;
                edgeToNode[newEdgeId] = {descendant, ancestor};

                if (adj[ancestor].first == node) 
                    adj[ancestor].first = descendant;
                else 
                    adj[ancestor].second = descendant;

                parent[descendant] = ancestor;

                parent[node] = -1;
                adj[node] = {-1,-1}; 

                updatePathsRemoving(descendantEdge);

            } else if (descendant == -1) {
                removeEdge(node, ancestor);
                removeNodeFromAdj(node);
            } else {
                removeEdge(descendant, node);
                adj[node] = {-1,-1};
                parent[descendant] = -1;
            }
        }   
    }
}

void MIPForest::addIncompatiblePathPartition(const MIPForest* F, int a, int b, int c, int d, std::unordered_set<Path, PathHash> &incompatible) {
    if (not pathIntersection(a,b,c,d) and F->pathIntersection(a,b,c,d)) {
        int id = incomPathSize.size();
        const int incompatiblePathSize = pathSize(a,b) + pathSize(c,d);
        
        incompatible.insert(Path(modId(),a, b, c, d));
        incomPathSize.push_back(incompatiblePathSize);

        for (int e: pathBetween(a,b))
            incompaiblePathPairsForEdge[e].push_back(id);

        for (int e: pathBetween(c,d))
            incompaiblePathPairsForEdge[e].push_back(id);

    }
        
    
    if (pathIntersection(a,b,c,d) and not F->pathIntersection(a,b,c,d))  
        incompatible.insert(Path(F->modId(),a, b, c, d));
}
