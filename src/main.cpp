#include "core/BTSolver.h"
#include "core/MIPSolver.h"

int main() {
    Instance instance = Instance();

    MIPSolver mipSolver = MIPSolver();

    mipSolver.solve(instance);
    
    return 0;
}
