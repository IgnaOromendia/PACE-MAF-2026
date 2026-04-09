#ifndef MIPMODEL_H
#define MIPMODEL_H

#include <ilcplex/ilocplex.h>
#include <memory>
#include "../model/forest/MIPForest.h"
#include <unordered_set>
#include <chrono>

class MIPModel {
protected:
    // Equations File
    std::string equationsFile = "src/data/equations";

    // Instance
    MIPForest* F1;
    MIPForest* F2;
    int run = 0;
    std::unordered_set<Triple, TripleHash> conflictiveTriples;
    std::unordered_set<Path, PathHash> incompatiblePaths;

    // Variables
    IloArray<IloIntVarArray> D;
    IloNumVarArray primalHeuristicVars;
    IloNumArray primalHeuristicVals;

    // CPLEX
    IloEnv env;
    IloCplex solver;
    IloModel model;

    // Model
    void initializeDVariableFor(MIPForest* F);

    // CPLEX helps
    void addConstraint(IloNum lhs, IloExpr& expr, IloNum rhs, std::string name);

public:
    MIPModel(): F1(nullptr), F2(nullptr) {};
    MIPModel(MIPForest* F1, MIPForest* F2);
    virtual ~MIPModel();
    MIPModel(const MIPModel&) = delete;
    MIPModel& operator=(const MIPModel&) = delete;
    MIPModel(MIPModel&&) = delete;
    MIPModel& operator=(MIPModel&&) = delete;

    virtual void generateVariables();
    virtual void setConstraints() = 0;
    virtual void setObjective() = 0;
    virtual void solve(bool exportModel = false) { cplexSolve(exportModel); };

    void cplexSolve(bool exportModel = false);

    void addPrimalHeuristic(const std::unordered_set<int>& edgesF1, const std::unordered_set<int>& edgesF2);
    int getValueFor(int forestId, int edgeId) const;
    virtual int getValueFor(int forestId, int i, int j) const;
    void exportSolution() const;
    bool isInfeasible() const;
};



#endif
