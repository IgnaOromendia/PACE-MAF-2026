#include "core/BTSolver.h"
#include "core/MIPSolver.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Input file missing\n";
        return 1;
    }

    std::string path = argv[1];

    Instance instance(path);
    // BTSolver solver = BTSolver(100000);

    // Forest* solution = solver.solve(instance);
    // if (solution == nullptr) {
    //     std::cerr << "Solver returned no solution\n";
    //     return 2;
    // }

    // solution->printAdjAndParents();

    MIPSolver mipSolver = MIPSolver();

    MIPForest* MAF = mipSolver.solve(instance);

    MAF->printAdjAndParents();

    instance.exportOutput(MAF);

    return 0;
}
