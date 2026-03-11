#include "ForestBT.h"

ForestBT::ForestBT(int nodeAmount, int labelAmount): Forest(nodeAmount, labelAmount) {
    this->nodesInfo.resize(nodeAmount);
}

ForestBT::ForestBT(std::vector<std::pair<int, int>> adjacency, std::vector<int> parents, int labelAmount): Forest(adjacency, parents, labelAmount) {
    this->nodesInfo.resize(nodeAmount);
}

ForestBT::ForestBT(const ForestBT &other): Forest(other) {
    nodesInfo = other.nodesInfo;
}

ForestBT::~ForestBT() {}

bool ForestBT::areSiblings(int a, int b) const {
    if (not nodeInRange(a) or not nodeInRange(b)) return false;
    return parent[a] != -1 && parent[a] == parent[b];
}

std::pair<int, int> ForestBT::siblings() const {
    for (int a = 0; a < labelsAmount; a++) 
        for(int b = a + 1; b < labelsAmount; b++) 
            if (areSiblings(a,b))
                return { a, b };

    return {-1, -1};
}

ForestBT *ForestBT::cut(int node) const {
    ForestBT* f = new ForestBT(*this);

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

ForestBT *ForestBT::shrink(int a, int b) const
{
    ForestBT* f = new ForestBT(*this);

    int p = f->parent[a];
    if (p == -1) return f;

    f->parent[a] = -1;
    f->parent[b] = -1;
    f->adj[p] = {-1, -1};
    f->nodesInfo[p] = NodeInfo(false, a, b);

    return f;
}

ForestBT *ForestBT::expand() {
    ForestBT* f = new ForestBT(*this);

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

ForestBT *ForestBT::prunePathBetween(int a, int b) const {
    ForestBT* f = new ForestBT(*this);
    int lca = f->LCA(a,b);

    f = walkAndPrune(f, a, lca);
    f = walkAndPrune(f, b, lca);

    return f;
}

ForestBT* ForestBT::walkAndPrune(ForestBT* f, int from, int to) const {
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