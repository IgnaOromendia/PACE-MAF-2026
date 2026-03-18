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

std::vector<int> MIPForest::pathBetween(int v, int w) const {
    std::vector<int> path;

    if (sameConnectedComponent(v,w)) {
        int u = LCA(v, w);
    
        walkAndAdd(v, u, path);

        std::vector<int> halfPath;

        walkAndAdd(w, u, halfPath);

        path.reserve(path.size() + halfPath.size());

        for(int i = halfPath.size() - 1; i >= 0; i--)
            path.push_back(halfPath[i]);
    }

    return path;
}

int MIPForest::pathSize(int v, int w) const {
    return pathBetween(v,w).size();
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

void MIPForest::walkAndAdd(int from, int to, std::vector<int>& path) const {
    int curr = parent[from];
    int prev = from;

    while(prev != to) {
        path.push_back(nodeToEdge.at({prev, curr}));
        prev = curr;
        curr = parent[curr];
    }
}

std::pair<int,int> MIPForest::low(const Triple& t) const {
    int l_ij = LCA(t.i, t.j);
    int l_ik = LCA(t.i, t.k);
    int l_jk = LCA(t.j, t.k);

    if (l_ij == l_ik) return {t.j, t.k};
    if (l_ij == l_jk) return {t.i, t.k};

    return {t.i, t.j};
}

void MIPForest::conflictiveTriples(const MIPForest* F, std::vector<Triple>& conflictive) const {
    for(int v = 0; v < labelAmount(); v++) {
        for(int w = v + 1; w < labelAmount(); w++) {
            if (not sameConnectedComponent(v,w)) continue;
            for(int z = w + 1; z < labelAmount(); z++) {
                if (not sameConnectedComponent(w,z)) continue;
                Triple t = Triple(v,w,z);
                if (low(t) != F->low(t))
                    conflictive.push_back(t);
            }
        }
    }       
}

bool MIPForest::isConflictive(const Triple &t, const MIPForest *F) const {
    if (not sameConnectedComponent(t.i, t.j) or not sameConnectedComponent(t.i, t.k) or not sameConnectedComponent(t.j, t.k)) return false;
    return low(t) != F->low(t);
}
