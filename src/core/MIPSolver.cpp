#include "MIPSolver.h"

#include <algorithm>
#include <iomanip>

MIPSolver::MIPSolver() {}

MIPSolver::~MIPSolver() {
    for(MIPForest* F: forests) delete F;
}

void MIPSolver::solve(Instance& instance) {
    forests = instance.mipTrees();

    size_t index = 1;

    MIPForest* F = forests.front();
    
    while(index < forests.size()) {
        // std::cout << "--------- " << index << " ---------\n";
        solveFor(F, forests[index++]);
    }

    instance.exportOutput(F);
}

void MIPSolver::solveFor(MIPForest* MAFCandidate, MIPForest* F) {
    // Inicializa modelo
    // mip = std::make_unique<PathMIPModel>(MAFCandidate, F);
    mip = std::make_unique<PairMIPModel>(MAFCandidate, F);

    // GreedySolver greedy = GreedySolver(MAFCandidate, F);
    // std::unordered_set<int> edgesF1 = greedy.edgesToCutF1(), edgesF2 = greedy.edgesToCutF2();

    // MIP generation
    mip->generateVariables();
    // mip->addPrimalHeuristic(edgesF1, edgesF2);
    mip->setConstraints();
    mip->setObjective();

    // Solve
    mip->solve();


    pruneAndRegraft(MAFCandidate);
}

void MIPSolver::pruneAndRegraft(MIPForest* F) const {

    // Cutting and Regraft
    for(int e = 0; e < F->amountOfEdges(); e++) {
        int cutting = mip->getValueFor(F->modId(), e);

        if (not cutting or not F->edgeIsAvailable(e)) continue;

        auto [descendant, ancestor] = F->nodesOf(e);

        F->cut(descendant);
    }

    F->regraft();

}
