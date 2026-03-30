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

std::pair<int,double> GreedySolver::bestEdgeOn(MIPForest* F) const {
    std::vector<int> freq(F->amountOfEdges(), 0);

    int bestEdge = -1;
    double bestScore = -1;

    for(int e = 0; e < F->amountOfEdges(); e++) {
        double score = F->edgeScore(e);
        if (bestScore < score) {
            bestScore = score;
            bestEdge = e;
        }
    }

    return {bestEdge, bestScore};
}

void GreedySolver::addAndCutEdge(int edgeId, MIPForest *F, std::unordered_set<int>& edgeSet) {
    auto[l, u] = F->nodesOf(edgeId);
    F->cut(l);
    edgeSet.insert(edgeId);
}

void GreedySolver::solve() {
    std::unordered_set<Triple, TripleHash> conflictiveTriples;
    std::unordered_set<Path, PathHash> incompatiblePaths;
    
    F1->conflictiveTriples(F2, conflictiveTriples);
    F1->incompatiblePaths(F2, incompatiblePaths);

    while (not conflictiveTriples.empty()) {
        auto [edgeF1, freqF1] = bestEdgeOn(F1);
        auto [edgeF2, freqF2] = bestEdgeOn(F2);

        if (freqF1 >= freqF2) 
            addAndCutEdge(edgeF1, F1, edgeSetF1);
        else
            addAndCutEdge(edgeF2, F2, edgeSetF2);

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
