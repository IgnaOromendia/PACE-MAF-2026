#include "GreedySolver.h"

#include <iostream>


GreedySolver::GreedySolver(MIPForest *A, MIPForest *B) {
    F1 = new MIPForest(*A);
    F2 = new MIPForest(*B);
    solve();
}

GreedySolver::~GreedySolver() {
    delete F1;
    delete F2;
}

std::pair<int,int> GreedySolver::maxFreqEdgeOn(MIPForest* F, std::vector<Triple>& conflictive) const {
    std::vector<int> freq(F->amountOfEdges(), 0);

    for(const Triple& t: conflictive) {
        for(int e: F->pathBetween(t.i, t.j)) 
                freq[e]++;
            
        for(int e: F->pathBetween(t.i, t.k)) 
            freq[e]++;
    }

    int bestEdge = -1, maxFreq = -1;

    for(int e = 0; e < F->amountOfEdges(); e++) {
        if (not F->edgeIsAvailable(e)) continue;
        if (freq[e] > maxFreq) {
            bestEdge = e;
            maxFreq = freq[e];
        }
    }

    return {bestEdge, maxFreq};
}

void GreedySolver::addAndCutEdge(int edgeId, MIPForest *F, std::unordered_set<int>& edgeSet) {
    auto[l, u] = F->nodesOf(edgeId);
    F->cut(l);
    edgeSet.insert(edgeId);
}

void GreedySolver::solve() {
    std::vector<Triple> conflictive;
    
    F1->conflictiveTriples(F2, conflictive);

    while (not conflictive.empty()) {
        auto [edgeF1, freqF1] = maxFreqEdgeOn(F1, conflictive);
        auto [edgeF2, freqF2] = maxFreqEdgeOn(F2, conflictive);

        if (freqF1 >= freqF2) 
            addAndCutEdge(edgeF1, F1, edgeSetF1);
        else
            addAndCutEdge(edgeF2, F2, edgeSetF2);

        F1->conflictiveTriples(F2, conflictive);
    }  
}

std::unordered_set<int> GreedySolver::edgesToCutF1() const {
    return edgeSetF1;
}

std::unordered_set<int> GreedySolver::edgesToCutF2() const {
    return edgeSetF2;
}
