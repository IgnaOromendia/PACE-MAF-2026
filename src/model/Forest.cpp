#include "Forest.h"

Forest::Forest(int nodeAmount, int labelAmount) {
    this->nodeAmount = nodeAmount;
    this->labelsAmount = labelAmount;
    this->treeCount = nodeAmount;
    this->rootId = labelAmount;
    
    this->adj.assign(nodeAmount, {-1, -1});
    this->parent.assign(nodeAmount, -1);

    this->tree.resize(nodeAmount);
    std::iota(this->tree.begin(), this->tree.end(), 0);

    this->nodesInfo.resize(nodeAmount);
}

Forest::Forest(std::vector<std::pair<int, int>> adj, std::vector<int> parents, int labelsAmount) {
    this->nodeAmount = adj.size();
    this->labelsAmount = labelsAmount;
    this->rootId = labelsAmount;

    this->adj = adj;
    this->parent = parents;
    this->nodesInfo.resize(nodeAmount);

    this->tree.assign(nodeAmount, -1);
    this->visited.assign(nodeAmount, 0);
    treeCount = 0;

    tree[rootId] = treeCount++;
    updateComponents(rootId);

    for(int v = 0; v < nodeAmount; v++) {
        if (not visited[v]) {
            tree[v] = treeCount;
            updateComponents(v);
            treeCount++;
        }
    }
}

Forest::Forest(const Forest& other) {
    nodeAmount = other.nodeAmount;
    labelsAmount = other.labelsAmount;
    treeCount = other.treeCount;
    adj = other.adj;
    parent = other.parent;
    tree = other.tree;
    nodesInfo = other.nodesInfo;
    rootId = other.rootId;
}

Forest::~Forest() {}

int Forest::labelAmount() const {
    return labelsAmount;
}

bool Forest::sameConnectedComponent(int a, int b) const {
    return tree[a] == tree[b];
}

int Forest::amountOfTrees() const {
    return treeCount;
}

int Forest::LCA(int a, int b) const {
    if (a < 0 or b < 0 or a >= nodeAmount or b >= nodeAmount) return -1;

    std::vector<char> isAncestor(nodeAmount, 0);

    int u = a;
    while (u != -1) {
        if (u < 0 or u >= nodeAmount) return -1;
        isAncestor[u] = 1;
        u = parent[u];
    }

    int v = b;
    while (v != -1) {
        if (v < 0 or v >= nodeAmount) return -1;
        if (isAncestor[v]) return v;
        v = parent[v];
    }

    return -1;
}

int Forest::root() const{
    return this->rootId;
}

int Forest::rootChild() const {
    return adj[rootId].first == -1 ? adj[rootId].second : adj[rootId].first;
}

void Forest::printAdjAndParents() const {
    std::cout << "node\t(left,right)\tparent\t\toriginal\tvirtualLabel\n";
    for (int i = 0; i < nodeAmount; i++) {
        std::cout << i << "\t(" << adj[i].first << "," << adj[i].second << ")" << "\t\t" << parent[i] << "\t\t";
        std::cout << nodesInfo[i].original << "\t\t(" << nodesInfo[i].left << "," << nodesInfo[i].right << ")" << "\n";
    }
}

bool Forest::areSiblings(int a, int b) const {
    if (not nodeInRange(a) or not nodeInRange(b)) return false;
    return parent[a] != -1 && parent[a] == parent[b];
}

std::pair<int, int> Forest::siblings() const {
    for (int a = 0; a < labelsAmount; a++) 
        for(int b = a + 1; b < labelsAmount; b++) 
            if (areSiblings(a,b))
                return { a, b };

    return {-1, -1};
}

Forest* Forest::cut(int node) const {
    Forest* f = new Forest(*this);

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

Forest* Forest::shrink(int a, int b) const {
    Forest* f = new Forest(*this);

    int p = f->parent[a];
    if (p == -1) return f;

    f->parent[a] = -1;
    f->parent[b] = -1;
    f->adj[p] = {-1, -1};
    f->nodesInfo[p] = NodeInfo(false, a, b);

    return f;
}

Forest *Forest::prunePathBetween(int a, int b) const {
    Forest* f = new Forest(*this);
    int lca = f->LCA(a,b);

    f = walkAndPrune(f, a, lca);
    f = walkAndPrune(f, b, lca);

    return f;
}

Forest *Forest::expand() {
    Forest* f = new Forest(*this);

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

void Forest::updateComponents(int v) {
    visited[v] = true;

    auto[u, w] = adj[v];

    if (u != -1) {
        if (not visited[u]) {
            tree[u] = tree[v];
            updateComponents(u);
        }
        
    }

    if (w != -1) {
        if (not visited[w]) {
            tree[w] = tree[v];
            updateComponents(w);
        }
    }
}

Forest* Forest::walkAndPrune(Forest* f, int from, int to) const {
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

bool Forest::nodeInRange(int a) const {
    return 0 <= a and a < nodeAmount;
}
