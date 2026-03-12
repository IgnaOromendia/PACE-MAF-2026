#include "Forest.h"

Forest::Forest(int nodeAmount, int labelAmount) {
    this->nodeAmount    = nodeAmount;
    this->labelsAmount  = labelAmount;
    this->treeCount     = nodeAmount;
    this->rootId        = labelAmount;
    
    this->adj.assign(nodeAmount, {-1, -1});
    this->parent.assign(nodeAmount, -1);

    this->tree.resize(nodeAmount);
    std::iota(this->tree.begin(), this->tree.end(), 0);

    
}

Forest::Forest(std::vector<std::pair<int, int>> adj, std::vector<int> parents, int labelsAmount) {
    this->nodeAmount    = adj.size();
    this->labelsAmount  = labelsAmount;
    this->rootId        = labelsAmount;

    this->adj = adj;
    this->parent = parents;

    this->tree.assign(nodeAmount, 0);
    this->visited.assign(nodeAmount, 0);
    treeCount = 1;
}

Forest::Forest(const Forest& other) {
    nodeAmount      = other.nodeAmount;
    labelsAmount    = other.labelsAmount;
    treeCount       = other.treeCount;
    adj             = other.adj;
    parent          = other.parent;
    tree            = other.tree;
    rootId          = other.rootId;
}

Forest::~Forest() {}

int Forest::labelAmount() const {
    return labelsAmount;
}

int Forest::amountOfTrees() const {
    return treeCount;
}

int Forest::amountOfNodes() const {
    return nodeAmount;
}

bool Forest::sameConnectedComponent(int a, int b) const {
    return tree[a] == tree[b];
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
    std::cout << "node\t(left,right)\tparent\n";
    for (int i = 0; i < nodeAmount; i++) 
        std::cout << i << "\t(" << adj[i].first << "," << adj[i].second << ")" << "\t\t" << parent[i] << "\n";
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



bool Forest::nodeInRange(int a) const {
    return 0 <= a and a < nodeAmount;
}
