#include "MIPForest.h"
#include <algorithm>
#include <cassert>
#include "iomanip"


MIPForest::MIPForest(const MIPForest &other): Forest(other) {
    conflictedTriplesForEdge    = other.conflictedTriplesForEdge;
    incompaiblePathPairsForEdge = other.incompaiblePathPairsForEdge;

    tripleSize      = other.tripleSize;
    incomPathSize   = other.incomPathSize;
}

MIPForest::MIPForest(const Forest &other): Forest(other) {
    conflictedTriplesForEdge.assign(edgesAmount, std::unordered_set<int>());
    incompaiblePathPairsForEdge.assign(edgesAmount, std::unordered_set<int>());
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

    double involvedMean = 0.0;

    for (int t : conflictedTriplesForEdge[e]) {
        int edgesInvolved = tripleSize[t];
        // involvedMean += edgesInvolved;
        score += ALPHA / (edgesInvolved * edgesInvolved);
    }

    // involvedMean /= conflictedTriplesForEdge[e].size();

    // std::cout << "Conflicted amount: " << conflictedTriplesForEdge[e].size() << "\n";
    // std::cout << "Involved mean: " << involvedMean << "\n";
    // std::cout << "TRIPLE SCORE: " << score << "\n";

    return score;
}

double MIPForest::pathsScore(int e) const {
    double score = 0.0;

    for (int p : incompaiblePathPairsForEdge[e]) {
        int edgesInvolved = incomPathSize[p];
        score += BETA / (edgesInvolved * edgesInvolved);
    }

    // std::cout << "PATH SCORE: " << score << "\n";

    return score;
}

double MIPForest::edgeScore(int e, MIPForest* F) const {
    double score = 0.0;

    if (not edgeAvailable[e]) return 0;

    const auto& [u, v] = nodesOf(e);
    const auto& [l1, l2] = childrenOf(v);

    // std::cout << std::fixed << std::setprecision(6);

    // std::cout << "EDGE: " << e << "\n";

    if (isLeaf(l1) and isLeaf(l2) and F->areSiblings(l1, l2)) return -10;

    // Rewards de edge that has least restriction coverage
    double tScore = triplesScore(e);
    double pScore = pathsScore(e);

    // std::cout << "DAMAGE: " << damage << "\n";

    score = tScore + pScore;

    if (score < 0) score = 0;

    if (isLeaf(u)) score += 0.1; // Tie break

    // std::cout << "SCORE: " << score <<  "\n--------\n";

    return score;
}

double MIPForest::edgeDamage(int e) const {
    const auto& [u, v] = nodesOf(e);
    int a = subtreeLeafs[u];
    int b = amountOfLabels() - a;
    return MU * a * b; 
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
    conflictedTriplesForEdge.assign(edgesAmount, std::unordered_set<int>());
    tripleSize.clear();
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

                    tripleSize.push_back(triplePathSize);

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
    incomPathSize.clear();
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
        incompatible.insert(Path(modId(),a, b, c, d));

        int id = incomPathSize.size();
        const int incompatiblePathSize = pathSize(a,b) + pathSize(c,d);
        
        incomPathSize.push_back(incompatiblePathSize);

        for (int e: pathBetween(a,b))
            incompaiblePathPairsForEdge[e].insert(id);

        for (int e: pathBetween(c,d))
            incompaiblePathPairsForEdge[e].insert(id);
    }
        
        
    if (pathIntersection(a,b,c,d) and not F->pathIntersection(a,b,c,d))  
        incompatible.insert(Path(F->modId(),a, b, c, d));
}
