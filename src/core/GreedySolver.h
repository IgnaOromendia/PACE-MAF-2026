#ifndef GREEDYSOLVER_H
#define GREEDYSOLVER_H

#include "../model/forest/MIPForest.h"

struct EdgeInfo {
    int child,parent;
    double edgeScore;
    double tripleScore;
    double pathScore;
    double damage;
    int isLeaf;
    EdgeInfo(int child, int parent, double es, double ts, double ps, double d, int isLeaf): child(child), parent(parent), edgeScore(es), tripleScore(ts), pathScore(ps), damage(d), isLeaf(isLeaf) {}
};

class GreedySolver {
private:

    MIPForest* F1;
    MIPForest* F2;
    std::unordered_set<int> edgeSetF1, edgeSetF2;

    std::pair<int,double> bestEdgeOn(MIPForest* F1, MIPForest* F2) const;
    void addAndCutEdge(int edgeId, MIPForest* F, MIPForest* F2, std::unordered_set<int>& edgeSet);
    void solve();

public:
    GreedySolver(MIPForest* F1, MIPForest* F2);
    ~GreedySolver();

    std::unordered_set<int> edgesToCutF1() const;
    std::unordered_set<int> edgesToCutF2() const;

    std::vector<EdgeInfo> edgesInfo;
};

#endif
