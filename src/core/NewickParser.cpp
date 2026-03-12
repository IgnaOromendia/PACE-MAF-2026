#include "NewickParser.h"

NewickParser::NewickParser(std::string stringTree, int labelsAmount) {
    this->stringTree = std::move(stringTree);
    this->stringTreeSize = this->stringTree.size();
    this->labelsAmount = labelsAmount;
    this->nodeId = labelsAmount;
    this->maxNodes = 2 * labelsAmount;
}

NewickParser::~NewickParser(){}

Forest* NewickParser::parse() {
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

    return new Forest(this->adj, this->parent, labelsAmount);
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
