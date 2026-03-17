#include "core/BTSolver.h"
#include "core/MIPSolver.h"

int main() {
    Instance instance = Instance();

    MIPSolver mipSolver = MIPSolver();

    MIPForest* MAF = mipSolver.solve(instance);

    instance.exportOutput(MAF);

    delete MAF;
    
    return 0;
}
