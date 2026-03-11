#ifndef BTSOLVER_H
#define BTSOLVER_H

#include "Forest.h"
#include "../core/Instance.h"

class BTSolver {
private:

    int k;
    
    std::vector<Forest*> forests;

    Forest* solveFor(Forest* F1, Forest* F2);

public:
    BTSolver(int k);
    ~BTSolver();

    Forest* minOrder(Forest* A, Forest* B) const;

    Forest* solve(Instance instance);

};


#endif
