#include "Forest.h"

Forest::Forest(int nodeAmount, int labelAmount) {
    this->nodes = nodeAmount;
    this->labelsAmount = labelAmount;
    this->treeCount = nodeAmount;
    
    this->adj.assign(nodeAmount, {-1, -1});
    this->parent.assign(nodeAmount, -1);

    this->tree.resize(nodeAmount);
    std::iota(this->tree.begin(), this->tree.end(), 0);

    this->labelInfo.resize(nodeAmount);
}

Forest::Forest(const std::vector<std::pair<int, int>>& adjacency,
               const std::vector<int>& parents,
               int labelAmount) {
    nodes = static_cast<int>(adjacency.size());
    labelsAmount = labelAmount;

    adj = adjacency;
    parent = parents;
    labelInfo.resize(nodes);

    tree.assign(nodes, -1);
    visited.assign(nodes, 0);
    treeCount = 0;

    std::vector<int> stack;
    for (int i = 0; i < nodes; i++) {
        if (tree[i] != -1) continue;

        tree[i] = treeCount;
        stack.push_back(i);

        while (not stack.empty()) {
            int v = stack.back();
            stack.pop_back();

            auto [left, right] = adj[v];
            int up = parent[v];

            if (left != -1 and tree[left] == -1) {
                tree[left] = treeCount;
                stack.push_back(left);
            }
            if (right != -1 and tree[right] == -1) {
                tree[right] = treeCount;
                stack.push_back(right);
            }
            if (up != -1 and tree[up] == -1) {
                tree[up] = treeCount;
                stack.push_back(up);
            }
        }

        treeCount++;
    }

    visited.clear();
}

Forest::Forest(const Forest& other) {
    nodes = other.nodes;
    labelsAmount = other.labelsAmount;
    treeCount = other.treeCount;
    adj = other.adj;
    parent = other.parent;
    tree = other.tree;
    labelInfo = other.labelInfo;
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
    if (a < 0 or b < 0 or a >= nodes or b >= nodes) return -1;

    std::vector<char> isAncestor(nodes, 0);

    int u = a;
    while (u != -1) {
        if (u < 0 or u >= nodes) return -1;
        isAncestor[u] = 1;
        u = parent[u];
    }

    int v = b;
    while (v != -1) {
        if (v < 0 or v >= nodes) return -1;
        if (isAncestor[v]) return v;
        v = parent[v];
    }

    return -1;
}

int Forest::rootChild() const {
    return adj[0].first == -1 ? adj[0].second : adj[1].first;
}

void Forest::printAdjAndParents() const {
    std::cout << "node\tadj(left,right)\tparent\n";
    for (int i = 0; i < nodes; i++) {
        std::cout << i
                  << "\t(" << adj[i].first << "," << adj[i].second << ")"
                  << "\t\t" << parent[i] << "\n";
    }
}

bool Forest::areSiblings(int a, int b) const {
    return parent[a] == parent[b];
}

std::pair<int, int> Forest::siblings() const {
    for (int a = 1; a < labelsAmount; a++) 
        for(int b = a + 1; b < labelsAmount; b++) 
            if (areSiblings(a,b))
                return { a, b };

    return {-1, -1};
}

Forest* Forest::cut(int node) const {
    Forest* f = new Forest(*this);

    int p = f->parent[node];

    if (f->adj[p].first == node) f->adj[p] = {-1, f->adj[p].second};
    if (f->adj[p].second == node) f->adj[p] = {f->adj[p].first, -1};

    f->parent[node] = -1;

    f->tree[node] = f->treeCount++;

    f->visited.assign(nodes, 0);

    f->updateComponents(node);

    return f;
}

Forest* Forest::shrink(int a, int b) const {
    Forest* f = new Forest(*this);

    int p = f->parent[a];

    f->adj[p] = {-1, -1};
    f->labelInfo[p] = LabelInfo(false, a, b);

    return f;
}

Forest *Forest::prunePathBetween(int a, int b) const {
    Forest* f = new Forest(*this);
    int lca = f->LCA(a,b);

    walkAndPrune(f, a, lca);

    walkAndPrune(f, b, lca);

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

void Forest::walkAndPrune(Forest* f, int from, int to) const {
    int prev = from;
    int cur = f->parent[from];
    while(cur != to) {
        int u = f->adj[cur].first == prev ? f->adj[cur].second : f->adj[cur].first;
        f = f->cut(u);
        prev = cur;
        cur = f->parent[cur];
    }
}
