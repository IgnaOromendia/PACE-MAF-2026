#include "MIPSolver.h"

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

    // MAFCandidate->printAdjAndParents();
    // MAFCandidate->printEdgeIds();
    // std::cout << "--\n";
    // F->printAdjAndParents();
    // F->printEdgeIds();

    // MIP generation
    mip->generateVariables();

    std::unordered_set<int> edgesF1, edgesF2;
    MIPForest H1(*MAFCandidate);
    MIPForest H2(*F);

    computeConflictiveEdges(&H1, &H2, edgesF1, edgesF2);
    mip->addPrimalHeuristic(edgesF1, edgesF2);

    mip->setConstraints();
    mip->setObjective();
    
    // Solve
    mip->solve();
    // mip->exportSolution();

    pruneAndRegraft(MAFCandidate);
    // MAFCandidate->printAdjAndParents();
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

void MIPSolver::computeConflictiveEdges(MIPForest* F1, MIPForest* F2, std::unordered_set<int>& edgesF1, std::unordered_set<int>& edgesF2) const {
    while(true) {
        std::vector<Triple> conflictive;

        F1->conflictiveTriples(F2, conflictive);

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
            if (not F1->edgeIsAvailable(e)) continue;
            if (freq1[e] > maxFreq) {
                bestEdge = e;
                maxFreq = freq1[e];
                bestTree = 0;
            }
        }

        for(int e = 0; e < F2->amountOfEdges(); e++) {
            if (not F2->edgeIsAvailable(e)) continue;
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

