#include "BTForest.h"

BTForest::BTForest(int forestId, int nodeAmount, int labelAmount): Forest(forestId, nodeAmount, labelAmount) {
    initializeNodeInfo(nodeAmount);
}

BTForest::BTForest(int forestId, std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount): Forest(forestId, adjacency, parents, labelAmount) {
    initializeNodeInfo(nodeAmount);
}

BTForest::BTForest(const Forest &other): Forest(other) {
    initializeNodeInfo(other.amountOfNodes());
}

BTForest::BTForest(const BTForest &other): Forest(other) {
    nodesInfo = other.nodesInfo;
}

BTForest::~BTForest() {}

bool BTForest::areSiblings(int a, int b) const {
    if (not nodeInRange(a) or not nodeInRange(b)) return false;
    return parent[a] != -1 && parent[a] == parent[b];
}

std::pair<int, int> BTForest::siblings() const {
    for (int a = 0; a < labelsAmount; a++) 
        for(int b = a + 1; b < labelsAmount; b++) 
            if (areSiblings(a,b))
                return { a, b };

    return {-1, -1};
}

BTForest* BTForest::cut(int node) const {
    BTForest* f = new BTForest(*this);

    int p = f->parent[node];

    if (p != -1) {
        if (f->adj[p].first == node) f->adj[p] = {-1, f->adj[p].second};
        if (f->adj[p].second == node) f->adj[p] = {f->adj[p].first, -1};
    }

    f->parent[node] = -1;

    f->tree[node] = f->treeCount++;

    f->visited.assign(nodeAmount, 0);

    f->updateComponents(node);

    return f;
}

BTForest* BTForest::shrink(int a, int b) const {
    BTForest* f = new BTForest(*this);

    int p = f->parent[a];
    if (p == -1) return f;

    f->parent[a] = -1;
    f->parent[b] = -1;
    f->adj[p] = {-1, -1};
    f->nodesInfo[p] = NodeInfo(p, a, b);

    return f;
}

BTForest* BTForest::expand() const {
    BTForest* f = new BTForest(*this);

    for(int i = 0; i < nodeAmount; i++) {
        if (f->nodesInfo[i].original) continue;
        
        int l = f->nodesInfo[i].left, r = f->nodesInfo[i].right;

        f->adj[i].first = l;
        f->adj[i].second = r;

        f->parent[l] = i;
        f->parent[r] = i;

        f->nodesInfo[i] = NodeInfo();
    }

    return f;
}

BTForest* BTForest::prunePathBetween(int a, int b) const {
    BTForest* f = new BTForest(*this);
    int lca = f->LCA(a,b);

    f = walkAndPrune(f, a, lca);
    f = walkAndPrune(f, b, lca);

    return f;
}

BTForest* BTForest::singletons() const {
    BTForest* f = new BTForest(*this);

    for(int v = labelsAmount; v < nodeAmount; v++) {
        f->parent[v] = -1;
        f->adj[v] = {-1,-1};
    }

    return f;
}

BTForest* BTForest::walkAndPrune(BTForest* f, int from, int to) const {
    int prev = from;
    int cur = f->parent[from];
    while(cur != to) {
        int u = f->adj[cur].first == prev ? f->adj[cur].second : f->adj[cur].first;
        f = f->cut(u);
        prev = cur;
        cur = f->parent[cur];
    }
    return f;
}

void BTForest::initializeNodeInfo(int amount) {
    this->nodesInfo.reserve(amount);
    for(int v = 0; v < amount; v++)
        this->nodesInfo.emplace_back(v);
}
