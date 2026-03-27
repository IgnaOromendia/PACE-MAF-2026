#include "Forest.h"
#include <algorithm>

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
    this->leafsForEdge.assign(edgesAmount, std::unordered_set<std::pair<int, int>, EdgeHash>());
    this->tin.assign(nodeAmount, 0);
    this->tout.assign(nodeAmount, 0);

    this->tree.resize(nodeAmount);
    std::iota(this->tree.begin(), this->tree.end(), 0);

    tagEdges();   
    precomputeAllPaths(); 
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
    this->tree.assign(nodeAmount, -1);
    this->visited.assign(nodeAmount, 0);
    this->leafsForEdge.assign(edgesAmount, std::unordered_set<std::pair<int, int>, EdgeHash>());
    this->tin.assign(nodeAmount, 0);
    this->tout.assign(nodeAmount, 0);

    treeCount = 0;
    timer = 0;
    
    for(int v = labelsAmount; v < nodeAmount; v++) { 
        if (nodeAvailable(v) and not visited[v]) {
            tree[v] = treeCount++;
            updateComponents(v);
        }       
    }

    for(int v = 0; v < labelsAmount; v++) { 
        if (nodeAvailable(v) and not visited[v]) {
            tree[v] = treeCount++;
            updateComponents(v);
        }       
    }
    
    tagEdges();
    precomputeAllPaths();
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
    paths           = other.paths;
    leafsForEdge    = other.leafsForEdge;
    tin             = other.tin;
    tout            = other.tout;
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

    if (ancestor == -1) return;

    if (adj[ancestor].first == node) adj[ancestor].first = -1;
    else adj[ancestor].second = -1;
}

int Forest::sibling(int node) const {
    if (parent[node] == -1) return -1;

    if (adj[node].first == node) 
        return adj[node].second < labelsAmount ? adj[node].second : -1;
    
    if (adj[node].second == node) 
        return adj[node].first < labelsAmount ? adj[node].first : -1;

    return -1;
}

int Forest::nextNodeInPathTo(int v, int w) const {
    int edgeId = pathBetween(v,w).front();
    return edgeToNode[edgeId].first == v ? edgeToNode[edgeId].second : edgeToNode[edgeId].first;
}

bool Forest::isAncestor(int v, int w) const {
    return tin[v] <= tin[w] and tout[w] <= tout[v];
}

bool Forest::onPath(int v, int i, int j) const {
    int l = LCA(i,j);
    return isAncestor(l, v) and (isAncestor(v, i) or isAncestor(v, j));
}

bool Forest::nodeAvailable(int node) const {
    if (node < labelsAmount) return true;
    return adj[node] != std::make_pair(-1,-1);
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

std::vector<int> Forest::pathBetween(int v, int w) const {
    if (not sameConnectedComponent(v,w)) return std::vector<int>();
    return paths.at({v,w});
}

int Forest::pathSize(int v, int w) const {
    return pathBetween(v,w).size();
}

int Forest::pathScore(int v, int w) const {
    int sum = 0;
    for(int e: pathBetween(v,w)) 
        sum += edgeScore(e);
    return sum;
}

int Forest::edgeForNode(int v, int w) const {
    int descendant = isAncestor(v,w) ? w : v;
    int ancestor = isAncestor(v,w) ? v : w;
    return nodeToEdge.at({descendant, ancestor});
}

bool Forest::pathIntersection(int i, int j, int k, int l) const {
    int l_ij = LCA(i,j);
    int l_kl = LCA(k,l);
    return onPath(l_ij, k, l) or onPath(l_kl, i, j);
}

int Forest::edgeScore(int e) const {
    return leafsForEdge[e].size();
}

void Forest::cut(int node) {
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

    timer = 0;
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

void Forest::printAll() const {
    std::cout << "Forest {\n";
    std::cout << "  forestId: " << forestId << "\n";
    std::cout << "  nodeAmount: " << nodeAmount << "\n";
    std::cout << "  labelsAmount: " << labelsAmount << "\n";
    std::cout << "  treeCount: " << treeCount << "\n";
    std::cout << "  rootId: " << rootId << "\n";
    std::cout << "  edgesAmount: " << edgesAmount << "\n";
    std::cout << "  timer: " << timer << "\n";
    std::cout << "}\n";

    std::cout << "Nodes\n";
    std::cout << "node\t(left,right)\tparent\ttree\tvisited\ttin\ttout\tleaf\n";
    for (int i = 0; i < nodeAmount; i++) {
        int visitedValue = i < static_cast<int>(visited.size()) ? visited[i] : -1;
        int tinValue = i < static_cast<int>(tin.size()) ? tin[i] : -1;
        int toutValue = i < static_cast<int>(tout.size()) ? tout[i] : -1;

        std::cout << i
                  << "\t(" << adj[i].first << "," << adj[i].second << ")"
                  << "\t\t" << parent[i]
                  << "\t" << tree[i]
                  << "\t" << visitedValue
                  << "\t" << tinValue
                  << "\t" << toutValue
                  << "\t" << isLeaf(i)
                  << "\n";
    }

    std::cout << "Edges\n";
    for (int edgeId = 0; edgeId < edgesAmount; edgeId++) {
        std::pair<int, int> nodes = edgeId < static_cast<int>(edgeToNode.size()) ? edgeToNode[edgeId] : std::pair<int, int>{-1, -1};
        bool available = edgeId < static_cast<int>(edgeAvailable.size()) ? edgeAvailable[edgeId] : false;

        std::cout << edgeId
                  << ": (" << nodes.first << "," << nodes.second << ")"
                  << " available=" << available
                  << "\n";
    }

    std::vector<std::pair<int, std::pair<int, int>>> nodeToEdgeEntries;
    nodeToEdgeEntries.reserve(nodeToEdge.size());
    for (const auto& [nodes, edgeId] : nodeToEdge)
        nodeToEdgeEntries.push_back({edgeId, nodes});

    std::sort(nodeToEdgeEntries.begin(), nodeToEdgeEntries.end());

    std::cout << "nodeToEdge\n";
    for (const auto& [edgeId, nodes] : nodeToEdgeEntries)
        std::cout << "(" << nodes.first << "," << nodes.second << ") -> " << edgeId << "\n";

    std::vector<std::pair<std::pair<int, int>, std::vector<int>>> sortedPaths;
    sortedPaths.reserve(paths.size());
    for (const auto& [nodes, path] : paths)
        sortedPaths.push_back({nodes, path});

    std::sort(
        sortedPaths.begin(),
        sortedPaths.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    std::cout << "Paths\n";
    for (const auto& [nodes, path] : sortedPaths) {
        std::cout << "(" << nodes.first << "," << nodes.second << "): [";
        for (int i = 0; i < static_cast<int>(path.size()); i++) {
            if (i > 0) std::cout << ",";
            std::cout << path[i];
        }
        std::cout << "]\n";
    }

    std::cout << "leafsForEdge\n";
    for (int edgeId = 0; edgeId < static_cast<int>(leafsForEdge.size()); edgeId++) {
        std::vector<std::pair<int, int>> leafPairs(leafsForEdge[edgeId].begin(), leafsForEdge[edgeId].end());
        std::sort(leafPairs.begin(), leafPairs.end());

        std::cout << edgeId << ": [";
        for (int i = 0; i < static_cast<int>(leafPairs.size()); i++) {
            if (i > 0) std::cout << ",";
            std::cout << "(" << leafPairs[i].first << "," << leafPairs[i].second << ")";
        }
        std::cout << "]\n";
    }
}

void Forest::updateComponents(int v) {
    visited[v] = true;
    tin[v] = timer++;

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

    tout[v] = timer++;
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

void Forest::walkAndAdd(int from, int lca, int to, std::vector<int> &path) {
    int curr = parent[from];
    int prev = from;

    while(prev != lca) {
        int edgeId = nodeToEdge.at({prev, curr});
        path.push_back(edgeId);
        leafsForEdge[edgeId].insert({std::min(from, to), std::max(from, to)});
        prev = curr;
        curr = parent[curr];
    }
}

void Forest::precomputeLeafPaths() {
    precomputePaths(labelsAmount);
}

void Forest::precomputeAllPaths() {
    precomputePaths(nodeAmount);
}

void Forest::precomputePaths(int limit) {
    for (int v = 0; v < limit; v++) {
        for (int w = v + 1; w < limit; w++) {
            std::vector<int> path;

            if (sameConnectedComponent(v,w)) {
                int u = LCA(v, w);
            
                walkAndAdd(v, u, w, path);

                std::vector<int> halfPath;

                walkAndAdd(w, u, v, halfPath);

                path.reserve(path.size() + halfPath.size());

                for(int i = halfPath.size() - 1; i >= 0; i--)
                    path.push_back(halfPath[i]);
            }

            paths.insert({{v,w}, path});
            paths.insert({{w,v}, path}); // puede q no sea necesario
        }
    }
}

void Forest::updatePathsRemoving(int descendantEdge) {
    std::unordered_set<std::pair<int, int>, EdgeHash> affectedLeafs = leafsForEdge[descendantEdge];

    for(auto& [v, u] : affectedLeafs) {
        if (not sameConnectedComponent(v,u)) continue;

        auto& path = paths.at({v, u});
        path.erase(std::remove(path.begin(), path.end(), descendantEdge), path.end());

        paths.insert({{u,v}, paths.at({v,u})}); // capaz no es necesario
    }
}
