#include "BTSolver.h"

BTSolver::BTSolver(int k): k(k) {}

BTSolver::~BTSolver() {}

BTForest* BTSolver::solveFor(BTForest* F1, BTForest* F2) {
    if (F1->amountOfTrees() > k) return nullptr;

    auto[a, b] = F2->siblings();

    if (a == -1 and b == -1) {
        int F2RootChild = F2->rootChild();

        if (F2RootChild != -1)
            if (F1->sameConnectedComponent(F2->root(), F2RootChild)) 
                return new BTForest(*F2);
            
        return F2->singletons();
    }

    if (not F1->sameConnectedComponent(a, b)) {

        BTForest* AFcutting_a = solveFor(F1->cut(a), F2->cut(a));
        BTForest* AFcutting_b = solveFor(F1->cut(b), F2->cut(b));

        return minOrder(AFcutting_a, AFcutting_b);
    } 

    if (F1->areSiblings(a,b)) {
        return solveFor(F1->shrink(a,b), F2->shrink(a,b));
    }

    if (F1->sameConnectedComponent(a,b) and not F1->areSiblings(a,b)) {

        BTForest* AFcutting_a = solveFor(F1->cut(a), F2->cut(a));
        BTForest* AFcutting_b = solveFor(F1->cut(b), F2->cut(b));
        BTForest* candidate = minOrder(AFcutting_a, AFcutting_b);

        BTForest* AFPrunning = solveFor(F1->prunePathBetween(a, b), F2);

        return minOrder(candidate, AFPrunning);
    } 

    return nullptr;
}

BTForest *BTSolver::minOrder(BTForest* A, BTForest* B) const {
    if (A == nullptr) return B;
    if (B == nullptr) return A;

    if (A->amountOfTrees() <= B->amountOfTrees()) {
        delete B;
        return A;
    }

    delete A;
    return B;
}

BTForest* BTSolver::solve(Instance instance) {
    std::vector<BTForest*> forest = instance.btTrees();
    BTForest* solution = solveFor(forest[0], forest[1]);
    if (solution == nullptr) return nullptr;
    return solution->expand();
}
