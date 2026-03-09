#include "core/Instance.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Input file missing\n";
        return 1;
    }

    std::string path = argv[1];

    Instance instance(path);

    return 0;
}