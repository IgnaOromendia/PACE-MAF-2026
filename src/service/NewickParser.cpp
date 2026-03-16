#include "NewickParser.h"
#include <stdexcept>
#include <utility>

NewickParser::NewickParser() {}

NewickParser::~NewickParser(){}

Forest* NewickParser::newickToForest(int id, std::string stringTree, int labelAmount) {
    this->stringTree        = std::move(stringTree);
    this->stringTreeSize    = this->stringTree.size();
    this->labelsAmount      = labelAmount;
    this->nodeId            = labelsAmount;
    this->maxNodes          = 2 * labelsAmount;
    this->index             = 0;

    this->adj.assign(maxNodes, {-1, -1});
    this->parent.assign(maxNodes, -1);

    parseNodeWithParent(-1);
    skipWhitespace();

    if (index < stringTreeSize and this->stringTree[index] == ';') {
        index++;
    } else {
        throw std::runtime_error("Invalid Newick: missing ';' terminator");
    }

    this->adj.resize(nodeId);
    this->parent.resize(nodeId);

    return new Forest(id, this->adj, this->parent, labelsAmount);
}

void NewickParser::forestToNewick(Forest* forest) {
    this->forest = forest;

    if (forest == nullptr) {
        throw std::runtime_error("Cannot serialize a null forest");
    }

    for (int v = 0; v < forest->amountOfNodes(); v++) {
        if (forest->parentOf(v) != -1) continue;

        std::string serialized = serializeNode(v);
        if (!serialized.empty()) 
            std::cout << serialized << ";\n";
        
    }
}

int NewickParser::parseNodeWithParent(int parentId) {
    skipWhitespace();
    if (index >= stringTreeSize) {
        throw std::runtime_error("Invalid Newick: unexpected end of input");
    }

    if (stringTree[index] == '(') {
        int currentNodeId = nodeId++;
        parent[currentNodeId] = parentId;

        index++; // consume '('
        int leftId = parseNodeWithParent(currentNodeId);

        skipWhitespace();
        if (index >= stringTreeSize || stringTree[index] != ',') {
            throw std::runtime_error("Invalid Newick: expected ','");
        }
        index++; // consume ','

        int rightId = parseNodeWithParent(currentNodeId);

        skipWhitespace();
        if (index >= stringTreeSize || stringTree[index] != ')') {
            throw std::runtime_error("Invalid Newick: expected ')'");
        }
        index++; // consume ')'

        adj[currentNodeId].first = leftId;
        adj[currentNodeId].second = rightId;

        return currentNodeId;
    }

    int label = getCurrentValue() - 1;
    parent[label] = parentId;
    return label;
}

int NewickParser::getCurrentValue() {
    skipWhitespace();
    if (index >= stringTreeSize || !std::isdigit(static_cast<unsigned char>(stringTree[index]))) {
        throw std::runtime_error("Invalid Newick: expected numeric label");
    }

    int value = 0;
    while (index < stringTreeSize and std::isdigit((unsigned char)stringTree[index])) {
        value = value * 10 + (stringTree[index] - '0');
        ++index;
    }
    return value;
}

void NewickParser::skipWhitespace() {
    while (index < stringTreeSize && std::isspace(static_cast<unsigned char>(stringTree[index]))) {
        ++index;
    }
}

std::string NewickParser::serializeNode(int node) const {
    if (forest->isLeaf(node)) {
        return node < labelsAmount ? std::to_string(node + 1) : "";
    } 

    auto [leftChild, rightChild] = forest->childrenOf(node);
    if (leftChild == -1 || rightChild == -1) {
        throw std::runtime_error("Invalid binary tree: internal node missing child");
    }

    return "(" + serializeNode(leftChild) + "," + serializeNode(rightChild) + ")";
}
