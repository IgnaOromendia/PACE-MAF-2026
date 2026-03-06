#include "NewickParser.h"

NewickParser::NewickParser() {}

NewickParser::~NewickParser(){}

Tree NewickParser::parse(std::string stringTree, int labels) {
    this->index = 0;
    this->nodesAmount = 0;
    this->stringTree = stringTree;

    Node* root = parseNode();
    
    return Tree(root, nodesAmount, labels);
}

Node* NewickParser::parseNode() {

    if (stringTree[index] == '(') {
        index++;
        Node* left = parseNode();

        index++; // consume ,

        Node* right = parseNode();

        index++; // consume )

        Node* node = new Node(left, right);
        this->nodesAmount++;

        return node;
    }

    int value = 0;
    while (index < stringTree.size() && isdigit((unsigned char)stringTree[index])) {
        value = value * 10 + (stringTree[index] - '0');
        ++index;
    }

    Node* leaf = new Node();
    leaf->label = value;
    this->nodesAmount++;
    return leaf;

}