#include "MIPForest.h"
#include <algorithm>
#include <cassert>
#include "iomanip"


MIPForest::MIPForest(const MIPForest &other): Forest(other) {
    conflictedTriplesForEdge    = other.conflictedTriplesForEdge;
    incompaiblePathPairsForEdge = other.incompaiblePathPairsForEdge;

    amountOfEdgesForTriple      = other.amountOfEdgesForTriple;
    amountOfEdgesForIncomPath   = other.amountOfEdgesForIncomPath;

    originalEdge = other.originalEdge;

    nodeContainer = other.nodeContainer;
}

MIPForest::MIPForest(const Forest &other): Forest(other) {
    conflictedTriplesForEdge.assign(edgesAmount, std::unordered_set<int>());
    incompaiblePathPairsForEdge.assign(edgesAmount, std::unordered_set<int>());
    originalEdge.assign(edgesAmount, 1);
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

double MIPForest::triplesScore(int e) const {
    double score = 0.0;

    for (int t : conflictedTriplesForEdge[e]) {
        int edgesInvolved = amountOfEdgesForTriple[t];
        score += ALPHA / (edgesInvolved * edgesInvolved);
    }

    return score;
}

double MIPForest::pathsScore(int e) const {
    double score = 0.0;

    for (int p : incompaiblePathPairsForEdge[e]) {
        int edgesInvolved = amountOfEdgesForIncomPath[p];
        score += BETA / (edgesInvolved * edgesInvolved);
    }

    return score;
}

double MIPForest::edgeScore(int e, MIPForest* F) const {
    double score = 0.0;

    if (not edgeAvailable[e]) return 0;

    const auto& [u, v] = nodesOf(e);
    const auto& [l1, l2] = childrenOf(v);

    if (isLeaf(l1) and isLeaf(l2) and F->areSiblings(l1, l2)) return -10;

    // Rewards de edge that has least restriction coverage
    double tScore = triplesScore(e);
    double pScore = pathsScore(e);

    score = tScore + pScore;

    if (score < 0) score = 0;

    // std::cout << "EDGE: " << e 
    //           << "\tSCORE:\t" << score
    //           << "\tTRIPLE:\t" << tScore
    //           << "\tPATH:\t" << pScore
    //           << "\n-------\n";

    return score;
}

double MIPForest::edgeDamage(int e) const {
    const auto& [u, v] = nodesOf(e);
    int a = subtreeLeafs[u];
    int b = amountOfLabels() - a;
    return MU * a * b; 
}

bool MIPForest::isOriginal(int e) const {
    return originalEdge[e];
}

std::pair<int,int> MIPForest::low(const Triple& t) const {
    int l_ij = LCA(t.i, t.j);
    int l_ik = LCA(t.i, t.k);
    int l_jk = LCA(t.j, t.k);

    if (l_ij == l_ik) return {t.j, t.k};
    if (l_ij == l_jk) return {t.i, t.k};

    return std::minmax(t.i, t.j);
}

void MIPForest::conflictiveTriples(const MIPForest* F, std::unordered_set<Triple, TripleHash>& conflictive) {
    conflictedTriplesForEdge.assign(edgesAmount, std::unordered_set<int>());
    amountOfEdgesForTriple.clear();
    conflictive.clear();

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

                    amountOfEdgesForTriple.push_back(triplePathSize);

                    for (int e: pathBetween(i,j))
                        conflictedTriplesForEdge[e].insert(id);
                    
                    for (int e: pathBetween(i,k))
                        conflictedTriplesForEdge[e].insert(id);
                }       
            }
        }
    }       
}

bool MIPForest::isConflictive(const Triple &t, const MIPForest *F) const {
    return low(t) != F->low(t);
}

int MIPForest::amountOfTriples() const {
    int n = labelsAmount;
    return n * (n-1) * (n-2) / 6;
}

int MIPForest::amountOfConflictiveTriplesPerEdge(int e) const {
    return conflictedTriplesForEdge[e].size();
}

void MIPForest::incompatiblePaths(const MIPForest *F, std::unordered_set<Path, PathHash> &incompatible) {
    incompaiblePathPairsForEdge.assign(edgesAmount, std::unordered_set<int>());
    amountOfEdgesForIncomPath.clear();
    incompatible.clear();

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

int MIPForest::amountOfIncompatiblePathsPerEdge(int e) const {
    return incompaiblePathPairsForEdge[e].size();
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
    subtreeLeafs.assign(nodeAmount, 0);

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
                originalEdge[newEdgeId] = 0;

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

void MIPForest::shrinkWith(MIPForest *F2) {
    int L = amountOfLabels();

    std::vector<std::vector<ShrinkInfo>> containsF1(amountOfNodes(), std::vector<ShrinkInfo>());
    std::vector<std::vector<ShrinkInfo>> containsF2(amountOfNodes(), std::vector<ShrinkInfo>());

    std::vector<std::pair<int,int>> adjF1 = adj;
    std::vector<std::pair<int,int>> adjF2 = F2->adj;

    std::vector<int> parentF1 = parent;
    std::vector<int> parentF2 = F2->parent;
    
    bool modified = true;

    activeLabel.assign(amountOfNodes(), true);

    while(modified) {
        modified = false;

        for(int a = 0; a < L; a++) {
            if (not activeLabel[a]) continue;
            for(int b = a + 1; b < L; b++) {
                if (not activeLabel[b]) continue;

                bool siblingsF1 = parentF1[a] == parentF1[b] and parentF1[a] != -1;
                bool siblingsF2 = parentF2[a] == parentF2[b] and parentF2[a] != -1;

                // Hace rotacion con el menor sibling y parent
                if (siblingsF1 and siblingsF2) {
                    modified = true;

                    modifyAdjecncy(a,b, adjF1, parentF1, containsF1);
                    modifyAdjecncy(a,b, adjF2, parentF2, containsF2);

                    activeLabel[b] = false;
                }
            }
        }
    }

    adj = adjF1;
    parent = parentF1;
    nodeContainer = containsF1;

    F2->adj = adjF2;
    F2->parent = parentF2;
    F2->nodeContainer = containsF2; 
}

void MIPForest::expand() {

    std::vector<int> newParent = parent;
    std::vector<std::pair<int, int>> newAdj = adj;

    newParent.resize(nodeAmount);
    newAdj.resize(nodeAmount);

    for(int node = 0; node < nodeAmount; node++) {

        while(not nodeContainer[node].empty()) {
            ShrinkInfo info = nodeContainer[node].back();
            nodeContainer[node].pop_back();

            int new_g = newParent[node];

            newParent[info.parent] = new_g;
            newAdj[info.parent] = {node, info.sibling};
            
            // newAdj[node] = {-1, -1};
            newParent[node] = info.parent;
            newParent[info.sibling] = info.parent;

            if (new_g != -1) {
                auto [l, r] = newAdj[new_g];
                newAdj[new_g] = l == node ? std::make_pair(info.parent, r) : std::make_pair(l, info.parent);
            }
        }

    }

    adj = newAdj;
    parent = newParent;
}

void MIPForest::addIncompatiblePathPartition(const MIPForest* F, int a, int b, int c, int d, std::unordered_set<Path, PathHash> &incompatible) {
    if (not pathIntersection(a,b,c,d) and F->pathIntersection(a,b,c,d)) {
        incompatible.insert(Path(modId(),a, b, c, d));

        int id = amountOfEdgesForIncomPath.size();
        const int incompatiblePathSize = pathSize(a,b) + pathSize(c,d);
        
        amountOfEdgesForIncomPath.push_back(incompatiblePathSize);

        for (int e: pathBetween(a,b))
            incompaiblePathPairsForEdge[e].insert(id);

        for (int e: pathBetween(c,d))
            incompaiblePathPairsForEdge[e].insert(id);
    }
        
        
    if (pathIntersection(a,b,c,d) and not F->pathIntersection(a,b,c,d))  
        incompatible.insert(Path(F->modId(),a, b, c, d));
}

void MIPForest::modifyAdjecncy(int a, int b, std::vector<std::pair<int, int>> &adj, std::vector<int> &parent, std::vector<std::vector<ShrinkInfo>> &contains) {
    int p = parent[a];
    int g = parent[p];

    // Guardamos info de la rotacion
    contains[a].emplace_back(p, b, g);

    // Modificamos los parents
    parent[a] = g;
    parent[b] = -1;
    parent[p] = -1;

    // Limpiamos el adj de p
    adj[p] = {-1, -1};

    if (g == -1) return;
    // Aplicamos la rotacion entre p y a
    auto [f,s] = adj[g];
    adj[g] = f == p ? std::make_pair(a, s) : std::make_pair(f, a);
}
