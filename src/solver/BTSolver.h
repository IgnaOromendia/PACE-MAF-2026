#ifndef BTSOLVER_H
#define BTSOLVER_H

#include "../model/BTForest.h"
#include "../core/Instance.h"

class BTSolver {
private:

    int k;
    
    std::vector<BTForest*> forests;

    BTForest* solveFor(BTForest* F1, BTForest* F2);

public:
    BTSolver(int k);
    ~BTSolver();

    BTForest* minOrder(BTForest* A, BTForest* B) const;

    BTForest* solve(Instance instance);

};


#endif
