#include "Instance.h"

Instance::Instance(std::string fileName) {
    std::ifstream inputFile(fileName);

    if (!inputFile) std::cerr << "Input file error\n";

    std::string line, tag, key, value;
    newickParser = NewickParser();

    while (std::getline(inputFile, line)) {
        trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        
        if (line.rfind("#s", 0) == 0) {
            iss >> tag >> key;

            getline(iss, value);

            trim(value);
            unquote(value);

            if (key == "idigest") this->idigest = value;
            if (key == "name") this->name = value;
            if (key == "desc") this->description = value;

        } else if (line.rfind("#p", 0) == 0) {
            iss >> tag >> treesAmount >> labelsAmount;
            newickTrees.reserve(treesAmount);
        } else if (line[0] == '#') {
            continue;
        } else {
            newickTrees.push_back(newickParser.newickToForest(newickTrees.size(), line, labelsAmount));
        }
    }

    std::cout << "RUNNING: " << this->name << std::endl;
}

Instance::~Instance() {}

std::vector<Forest*> Instance::trees() const {
    return this->newickTrees;
}

std::vector<BTForest*> Instance::btTrees() const {
    std::vector<BTForest*> result = std::vector<BTForest*>();

    for(Forest* f: newickTrees)
        result.push_back(new BTForest(*f));
    
    return result;
}

std::vector<MIPForest*> Instance::mipTrees() const{
    std::vector<MIPForest*> result = std::vector<MIPForest*>();

    for(Forest* f: newickTrees)
        result.push_back(new MIPForest(*f));
    
    return result;
}

void Instance::exportOutput(Forest* forest) {
    newickParser.forestToNewick(forest, outputPath + "out_" + name + ".nw");
}

void Instance::unquote(std::string& text) {
    if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
        text.pop_back();
        text.erase(0,1);
    }
}

void Instance::trim(std::string& text) {
    const std::string whitespace = " \t\n\r\f\v";
    const size_t start = text.find_first_not_of(whitespace);

    if (start == std::string::npos) {
        text.clear();
        return;
    }

    const size_t end = text.find_last_not_of(whitespace);
    text = text.substr(start, end - start + 1);
}
