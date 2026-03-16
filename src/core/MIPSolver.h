#ifndef MIPSOLVER_H
#define MIPSOLVER_H

#include <ilcplex/ilocplex.h>
#include "../service/Instance.h"
#include "../model/MIPForest.h"

class MIPSolver {
private:
    // File
    std::string equationsFile = "src/data/equations.lp";

    // Instance
    std::vector<MIPForest*> forests;

    // Variables
    IloArray<IloIntVarArray> D;

    // CPLEX
    IloEnv env;
    IloCplex solver;
    IloModel model;

    MIPForest* solveFor(MIPForest* F1, MIPForest* F2);
    void generateVariablesWith(MIPForest* F1, MIPForest* F2);
    void initializeVariableFor(MIPForest* F);
    void setPathConstraints(MIPForest* F1, MIPForest* F2);
    void setLowLeafConstraints(MIPForest* F1,  MIPForest* F2);
    void addConstraint(IloNum lhs, IloExpr& expre, IloNum rhs, std::string name);
    void setObjective(MIPForest* F);
    void solve();
    void exportSolution() const;
    bool isConflictedLeaf(Triple t, MIPForest* F1, MIPForest* F2) const;
    MIPForest* pruneAndRegraft(MIPForest* F) const;
    void clearModel();

public:
    MIPSolver();
    ~MIPSolver();

    MIPForest* solve(Instance instance);
};


#endif