#include "core/BTSolver.h"
#include "core/MIPSolver.h"

int main() {

    Instance instance = Instance();
    // BTSolver solver = BTSolver(100000);

    // Forest* solution = solver.solve(instance);
    // if (solution == nullptr) {
    //     std::cerr << "Solver returned no solution\n";
    //     return 2;
    // }

    // solution->printAdjAndParents();

    MIPSolver mipSolver = MIPSolver();

    MIPForest* MAF = mipSolver.solve(instance);

    instance.exportOutput(MAF);

    return 0;
}
