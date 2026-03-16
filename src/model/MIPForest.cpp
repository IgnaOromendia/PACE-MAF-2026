#include "MIPForest.h"
#include <algorithm>

MIPForest::MIPForest(int forestId, int nodeAmount, int labelAmount) : Forest(forestId, nodeAmount, labelAmount) {
    tagEdges();
    precomputPaths();
}

MIPForest::MIPForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount): Forest(forestId, adjacency, parents, labelAmount) {
    tagEdges();
    precomputPaths();
}

MIPForest::MIPForest(const MIPForest &other): Forest(other) {
    this->edgeToNode = other.edgeToNode;
    this->nodeToEdge = other.nodeToEdge;
    this->paths = other.paths;
    this->edgesAmount = other.edgesAmount;
}

MIPForest::MIPForest(const Forest &other): Forest(other) {
    tagEdges();
    precomputPaths();
}

MIPForest::~MIPForest() {}

int MIPForest::modId() const {
    return id() % 2;
}

int MIPForest::amountOfEdges() const {
    return this->edgesAmount;
}

std::vector<int> MIPForest::pathBetween(int v, int w) const {
    return paths.at({v,w});
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

std::pair<int, int> MIPForest::nodesOf(int edgeId) const {
    return edgeToNode[edgeId];
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

void MIPForest::precomputPaths() {
    for (int v = 0; v < nodeAmount; v++) {
        if (not isLeaf(v)) continue;

        for(int w = v + 1; w < nodeAmount; w++) {
            if (not isLeaf(w)) continue;
            
            std::vector<int> path, halfPath;
            int u = LCA(v, w);
            
            walkAndAdd(v, u, path);

            walkAndAdd(w, u, halfPath);

            path.reserve(path.size() + halfPath.size());

            for(int i = halfPath.size() - 1; i >= 0; i--)
                path.push_back(halfPath[i]);

            paths.insert({{v,w}, path});
            paths.insert({{w,v}, path});
        }
    }
}

void MIPForest::tagEdges() {
    int edgeCount = 0;
    edgeToNode.reserve(nodeAmount - 1);
    nodeToEdge.reserve(nodeAmount - 1);
    for(int v = 0; v < nodeAmount; v++){
        if (parent[v] == -1) continue;
        edgeToNode.push_back({v, parent[v]});
        nodeToEdge.insert({{v, parent[v]}, edgeCount++});
    }
    this->edgesAmount = edgeCount;
}

std::pair<int,int> MIPForest::low(Triple t) const {
    int l_ij = LCA(t.i, t.j);
    int l_ik = LCA(t.i, t.k);
    int l_jk = LCA(t.j, t.k);

    if (l_ij == l_ik) return {t.j, t.k};
    if (l_ij == l_jk) return {t.i, t.k};

    return {t.i, t.j};
}
