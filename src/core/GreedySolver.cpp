#include "GreedySolver.h"

GreedySolver::GreedySolver(MIPForest *A, MIPForest *B) {
    F1 = new MIPForest(*A);
    F2 = new MIPForest(*B);
    solve();
}

GreedySolver::~GreedySolver() {
    delete F1;
    delete F2;
}

std::pair<int,double> GreedySolver::bestEdgeOn(MIPForest* F1, MIPForest* F2) const {
    int bestEdge = -2;
    double bestScore = -1;

    for(int e = 0; e < F1->amountOfEdges(); e++) {
        double score = F1->edgeScore(e, F2);
        if (bestScore < score) {
            bestScore = score;
            bestEdge = e;
        }
    }

    return {bestEdge, bestScore};
}

void GreedySolver::addAndCutEdge(int edgeId, MIPForest *F, MIPForest* F2, std::unordered_set<int>& edgeSet) {
    // std::cout << "F" << F->modId() << "\n";
    // F->printAdjAndParents();
    // F->printEdgeIds();
    // std::cout << "CUTTING  -------------------------------------\n"; 
    auto[l, u] = F->nodesOf(edgeId);
    // std::cout << edgeId << ": " << l << " " << u << "\n";
    edgesInfo.emplace_back(l, u, F->edgeScore(edgeId, F2), edgesInfo.size(), F->triplesScore(edgeId), F->pathsScore(edgeId), F->edgeDamage(edgeId), F->isLeaf(l));
    F->cut(l);
    F->regraft();
    edgeSet.insert(edgeId);
}

void GreedySolver::solve() {
    std::unordered_set<Triple, TripleHash> conflictiveTriples;
    std::unordered_set<Path, PathHash> incompatiblePaths;
    
    F1->conflictiveTriples(F2, conflictiveTriples);
    F1->incompatiblePaths(F2, incompatiblePaths);

    while (not conflictiveTriples.empty() and not incompatiblePaths.empty()) {
        // std::cout << "SEARCHING ----------------------\n";

        auto [edgeF1, scoreF1] = bestEdgeOn(F1, F2);
        // auto [edgeF2, scoreF2] = bestEdgeOn(F2, F1);

        // std::cout << edgeF1 << " " << scoreF1 << "\n";
        
        addAndCutEdge(edgeF1, F1, F2, edgeSetF1);
        // addAndCutEdge(edgeF2, F2, F1, edgeSetF2);

        // std::cout << "cutted\n"; 

        F1->conflictiveTriples(F2, conflictiveTriples);
        F1->incompatiblePaths(F2, incompatiblePaths);
    }  
}

std::unordered_set<int> GreedySolver::edgesToCutF1() const {
    return edgeSetF1;
}

std::unordered_set<int> GreedySolver::edgesToCutF2() const {
    return edgeSetF2;
}
