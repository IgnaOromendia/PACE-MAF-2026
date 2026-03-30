#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "../model/forest/BTForest.h"
#include "../model/forest/MIPForest.h"
#include "NewickParser.h"

class Instance {
private:
    std::string name, idigest, description;

    int treesAmount, labelsAmount;
    
    std::vector<Forest*> newickTrees;
    NewickParser newickParser;

    void unquote(std::string& text);
    void trim(std::string& text);

public:
    Instance();
    ~Instance();

    std::vector<Forest*> trees() const;
    std::vector<BTForest*> btTrees() const;
    std::vector<MIPForest*> mipTrees() const;
    
    std::string title() const;

    void exportOutput(Forest* forest);
};

#endif