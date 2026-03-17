#include "MIPSolver.h"

MIPSolver::MIPSolver() {}

MIPSolver::~MIPSolver() {
    for(MIPForest* F: forests) delete F;
}

MIPForest* MIPSolver::solve(Instance& instance) {
    forests = instance.mipTrees();

    size_t index = 1;

    MIPForest* F = forests.front();
    
    while(index < forests.size()) 
        F = solveFor(F, forests[index++]);
    
    return F;
}

MIPForest* MIPSolver::solveFor(MIPForest* F1, MIPForest* F2) {
    // Inicializa modelo
    mip = std::make_unique<MIPModel>(F1, F2);

    // MIP generation
    mip->generateVariables();

    // std::unordered_set<int> edgesF1, edgesF2;
    // computeConflictiveEdges(F1, F2, edgesF1, edgesF2);
    // addPrimalHeuristic(edgesF1, edgesF2);

    mip->setPathConstraints();
    mip->setLowLeafConstraints();
    mip->setObjective();
    
    // Solve
    mip->solve();

    MIPForest* MAF = pruneAndRegraft(F1);

    return MAF;
}

MIPForest* MIPSolver::pruneAndRegraft(MIPForest* F) const {
    MIPForest* newForest = new MIPForest(*F);

    // Cutting and Regraft
    for(int e = 0; e < newForest->amountOfEdges(); e++) {
        int cutting = mip->getValueFor(newForest->modId(), e);
        
        if (not cutting) continue;

        auto [descendant, ancestor] = newForest->nodesOf(e);

        newForest->cut(descendant);
    }

    newForest->regraft();

    return newForest;
}

void MIPSolver::computeConflictiveEdges(MIPForest* F1, MIPForest* F2, std::unordered_set<int>& edgesF1, std::unordered_set<int>& edgesF2) const {
    while(true) {
        std::vector<Triple> conflictive;

        F1->conflictiveTriples(F2, conflictive);

        std::cout << conflictive.size() << "\n";

        if (conflictive.empty()) break;

        std::vector<int> freq1(F1->amountOfEdges(), 0);
        std::vector<int> freq2(F2->amountOfEdges(), 0);

        for(const Triple& t: conflictive) {
            for(int e: F1->pathBetween(t.i, t.j)) 
                freq1[e]++;
            
            for(int e: F1->pathBetween(t.i, t.k)) 
                freq1[e]++;

            for(int e: F2->pathBetween(t.i, t.j)) 
                freq2[e]++;
            
            for(int e: F2->pathBetween(t.i, t.k)) 
                freq2[e]++;
        }

        int bestTree = -1, bestEdge = -1, maxFreq = -1;

        for(int e = 0; e < F1->amountOfEdges(); e++) {
            if (freq1[e] > maxFreq) {
                bestEdge = e;
                maxFreq = freq1[e];
                bestTree = 0;
            }
        }

        for(int e = 0; e < F2->amountOfEdges(); e++) {
            if (freq2[e] > maxFreq) {
                bestEdge = e;
                maxFreq = freq2[e];
                bestTree = 1;
            }
        }

        if (bestTree == 0) {
            auto[l, u] = F1->nodesOf(bestEdge);
            F1->cut(l);
            edgesF1.insert(bestEdge);
        } else {
            auto[l, u] = F2->nodesOf(bestEdge);
            F2->cut(l);
            edgesF2.insert(bestEdge);
        }
    }
}

