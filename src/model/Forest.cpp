#include "Forest.h"

Forest::Forest(int forestId, int nodeAmount, int labelAmount) {
    this->forestId      = forestId;
    this->nodeAmount    = nodeAmount;
    this->labelsAmount  = labelAmount;
    this->treeCount     = nodeAmount;
    this->rootId        = labelAmount;
    this->edgesAmount   = nodeAmount - 1;
    
    this->adj.assign(nodeAmount, {-1, -1});
    this->parent.assign(nodeAmount, -1);
    this->edgeAvailable.assign(edgesAmount, true);

    this->tree.resize(nodeAmount);
    std::iota(this->tree.begin(), this->tree.end(), 0);

    tagEdges();    
}

Forest::Forest(int forestId, std::vector<std::pair<int, int>> adj, std::vector<int> parents, int labelsAmount) {
    this->forestId      = forestId;
    this->nodeAmount    = adj.size();
    this->labelsAmount  = labelsAmount;
    this->rootId        = labelsAmount;
    this->edgesAmount   = nodeAmount - 1;

    this->adj = adj;
    this->parent = parents;

    this->edgeAvailable.assign(edgesAmount, true);
    this->tree.assign(nodeAmount, 0);
    this->visited.assign(nodeAmount, 0);

    treeCount = 1;

    tagEdges();
}

Forest::Forest(const Forest& other) {
    forestId        = other.forestId;
    nodeAmount      = other.nodeAmount;
    labelsAmount    = other.labelsAmount;
    treeCount       = other.treeCount;
    adj             = other.adj;
    parent          = other.parent;
    tree            = other.tree;
    rootId          = other.rootId;
    edgeAvailable   = other.edgeAvailable;
    edgesAmount     = other.edgesAmount;
    edgeToNode      = other.edgeToNode;
    nodeToEdge      = other.nodeToEdge;
}

Forest::~Forest() {}

int Forest::id() const {
    return forestId;
}

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

bool Forest::isLeaf(int a) const {
    return adj[a].first == -1 and adj[a].second == -1;
}

std::pair<int, int> Forest::childrenOf(int node) const {
    return adj[node];
}

int Forest::parentOf(int node) const {
    return parent[node];
}

void Forest::removeNodeFromAdj(int node) {
    int ancestor = parent[node];

    parent[node] = -1;

    if (adj[ancestor].first == node) adj[ancestor].first = -1;
    else adj[ancestor].second = -1;
}

int Forest::amountOfEdges() const {
    return edgesAmount;
}

std::pair<int, int> Forest::nodesOf(int edgeId) const {
    return edgeToNode[edgeId];
}

bool Forest::edgeIsAvailable(int edgeId) const {
    return edgeAvailable[edgeId];
}

void Forest::removeEdge(int v, int u) {
    int edgeId = nodeToEdge.at({v,u});
    nodeToEdge.erase({v, u});
    edgeToNode[edgeId] = {-1,-1};
    edgeAvailable[edgeId] = false;
}

void Forest::cut(int node) {
    int p = parent[node];

    if (p != -1) {
        if (adj[p].first == node) adj[p] = {-1, adj[p].second};
        if (adj[p].second == node) adj[p] = {adj[p].first, -1};
    }


    int edgeId = nodeToEdge.at({node, p});
    edgeAvailable[edgeId] = false;

    parent[node] = -1;

    tree[node] = treeCount++;

    visited.assign(nodeAmount, 0);

    updateComponents(node);
}

void Forest::regraft() {
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
                int newEdgeId = nodeToEdge.at({node, ancestor});

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

void Forest::tagEdges() {
    int edgeCount = 0;
    edgeToNode.reserve(edgesAmount);
    nodeToEdge.reserve(edgesAmount);
    for(int v = 0; v < nodeAmount; v++){
        if (parent[v] == -1) continue;
        edgeToNode.push_back({v, parent[v]});
        nodeToEdge.insert({{v, parent[v]}, edgeCount++});
    }
}
