#include "GreedySolver.h"

GreedySolver::GreedySolver(MIPForest *A, MIPForest *B) {
    F1 = new MIPForest(*A);
    F2 = new MIPForest(*B);
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
    // edgesInfo.emplace_back(l, u, F->edgeScore(edgeId, F2), edgesInfo.size(), F->triplesScore(edgeId), F->pathsScore(edgeId), F->edgeDamage(edgeId), F->isLeaf(l));
    F->cut(l);
    F->regraft();
    if (F->isOriginal(edgeId)) edgeSet.insert(edgeId);
}

void GreedySolver::solveUsingConstraintsScore() {
    std::unordered_set<Triple, TripleHash> conflictiveTriples;
    std::unordered_set<Path, PathHash> incompatiblePaths;
    
    F1->conflictiveTriples(F2, conflictiveTriples);
    F1->incompatiblePaths(F2, incompatiblePaths);

    while (not conflictiveTriples.empty() and not incompatiblePaths.empty()) {
        auto [edgeF1, scoreF1] = bestEdgeOn(F1, F2);
        
        addAndCutEdge(edgeF1, F1, F2, edgeSetF1);

        F1->conflictiveTriples(F2, conflictiveTriples);
        F1->incompatiblePaths(F2, incompatiblePaths);
    }  
}

void GreedySolver::solveUsingSiblingsScore() {
    bool existsSiblings = true;

    while(existsSiblings) {
        existsSiblings = false;
        std::vector<int> score(F2->amountOfEdges(), 0);

        for(int a = 0; a < F2->amountOfLabels(); a++) {
            for(int b = a + 1; b < F2->amountOfLabels(); b++) {
                if (not F2->areSiblings(a,b)) continue;
                
                if (not F1->sameConnectedComponent(a,b)) {
                    int p_a = F1->parentOf(a);
                    int p_b = F1->parentOf(b);

                    int e1 = F1->edgeForNode(a, p_a);
                    int e2 = F1->edgeForNode(b, p_b);

                    score[e1] += 1;
                    score[e2] += 1;
                } else if (not F1->areSiblings(a,b) and F1->sameConnectedComponent(a,b)) {
                    int p_a = F1->parentOf(a);
                    int p_b = F1->parentOf(b);

                    int e1 = F1->edgeForNode(a, p_a);
                    int e2 = F1->edgeForNode(b, p_b);

                    score[e1] += 1;
                    score[e2] += 1;

                    for(int e: F1->pathBetween(a, b)) {
                        score[e] += 1;
                    }
                }

            }
        }

        int bestEdge = -1, bestScore = -1;

        for(int e = 0; e < F2->amountOfEdges(); e++) {
            if (score[e] > bestScore) {
                bestScore = score[e];
                bestEdge = e;
            }
        }

        addAndCutEdge(bestEdge, F1, F2, edgeSetF1);
    }
    
}

std::unordered_set<int> GreedySolver::edgesToCutF1() const {
    return edgeSetF1;
}

std::unordered_set<int> GreedySolver::edgesToCutF2() const {
    return edgeSetF2;
}
