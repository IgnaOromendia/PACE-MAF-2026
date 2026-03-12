#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "../model/BTForest.h"
#include "../model/MIPForest.h"

class Instance {
private:
    std::string name;
    std::string idigest;
    std::string description;

    int treesAmount, labelsAmount;
    
    std::vector<Forest*> newickTrees;

    void unquote(std::string& text);
    void trim(std::string& text);

public:
    Instance(std::string fileName);
    ~Instance();

    std::vector<Forest*> trees() const;
    std::vector<BTForest*> btTrees() const;
    std::vector<MIPForest*> mipTrees() const;
};

#endif