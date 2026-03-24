#ifndef GREEDYSOLVER_H
#define GREEDYSOLVER_H

#include "../model/MIPForest.h"

class GreedySolver {
private:

    MIPForest* F1;
    MIPForest* F2;
    std::unordered_set<int> edgeSetF1, edgeSetF2;

    std::pair<int,int> maxFreqEdgeOn(MIPForest* F, std::vector<Triple>& conflictive) const;
    void addAndCutEdge(int edgeId, MIPForest* F, std::unordered_set<int>& edgeSet);
    void solve();

public:
    GreedySolver(MIPForest* F1, MIPForest* F2);
    ~GreedySolver();

    std::unordered_set<int> edgesToCutF1() const;
    std::unordered_set<int> edgesToCutF2() const;
};

#endif
