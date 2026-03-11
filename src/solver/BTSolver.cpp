#include "BTSolver.h"

BTSolver::BTSolver(int k): k(k) {}

BTSolver::~BTSolver() {}

Forest* BTSolver::solveFor(Forest* F1, Forest* F2) {
    if (F1->amountOfTrees() > k) return nullptr;

    auto[a, b] = F2->siblings();

    if (a == -1 and b == -1) {
        int F2RootChild = F2->rootChild();

        if (F2RootChild != -1)
            if (F1->sameConnectedComponent(F2->root(), F2RootChild)) 
                return new Forest(*(F2->expand()));
            
        return new Forest(F2->labelAmount()); // Acá debería expandir los contridos también
    }

    if (not F1->sameConnectedComponent(a, b)) {

        Forest* AFcutting_a = solveFor(F1->cut(a), F2->cut(a));
        Forest* AFcutting_b = solveFor(F1->cut(b), F2->cut(b));

        return minOrder(AFcutting_a, AFcutting_b);
    } 

    if (F1->areSiblings(a,b)) {
        return solveFor(F1->shrink(a,b), F2->shrink(a,b));
    }

    if (F1->sameConnectedComponent(a,b) and not F1->areSiblings(a,b)) {

        Forest* AFcutting_a = solveFor(F1->cut(a), F2->cut(a));
        Forest* AFcutting_b = solveFor(F1->cut(b), F2->cut(b));
        Forest* candidate = minOrder(AFcutting_a, AFcutting_b);

        Forest* AFPrunning = solveFor(F1->prunePathBetween(a, b), F2);

        return minOrder(candidate, AFPrunning);
    } 

    return nullptr;
}

Forest *BTSolver::minOrder(Forest* A, Forest* B) const {
    if (A == nullptr) return B;
    if (B == nullptr) return A;
    return (A->amountOfTrees() <= B->amountOfTrees()) ? A : B;
}

Forest* BTSolver::solve(Instance instance) {
    Forest* F1 = instance.trees()[0];
    Forest* F2 = instance.trees()[1];
    return solveFor(F1, F2);
}
