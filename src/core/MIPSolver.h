#ifndef MIPSOLVER_H
#define MIPSOLVER_H

#include "../model/MIPModel.h"

class MIPSolver {
private:
    // Instance
    std::vector<MIPForest*> forests;

    // Model
    std::unique_ptr<MIPModel> mip;

    MIPForest* solveFor(MIPForest* F1, MIPForest* F2);
    MIPForest* pruneAndRegraft(MIPForest* F) const;

    // Primal
    void computeConflictiveEdges(MIPForest* F1, MIPForest* F2, std::unordered_set<int>& edgesF1, std::unordered_set<int>& edgesF2) const;

public:
    MIPSolver();
    ~MIPSolver();

    MIPForest* solve(Instance& instance);
};


#endif
