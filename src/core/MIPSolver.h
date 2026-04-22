#ifndef MIPSOLVER_H
#define MIPSOLVER_H

#include "../model/PathMIPModel.h"
#include "../model/PairMIPModel.h"
#include "../service/Instance.h"
#include "GreedySolver.h"

class MIPSolver {
private:
    std::string instanceName;

    // Instance
    std::vector<MIPForest*> forests;

    // Model
    std::unique_ptr<MIPModel> mip;

    void solveFor(MIPForest* F1, MIPForest* F2);
    void pruneAndRegraft(MIPForest* F) const;
    void writeEdgeStats(MIPForest* F, std::vector<EdgeInfo>& edgesInfo) const;

public:
    MIPSolver();
    ~MIPSolver();

    void solve(Instance& instance);
};


#endif
