#ifndef MIPMODEL_H
#define MIPMODEL_H

#include <ilcplex/ilocplex.h>
#include <memory>
#include "../model/MIPForest.h"
#include <unordered_set>
#include <chrono>

class MIPModel {
private:
    // Equations File
    std::string equationsFile = "src/data/equations";

    // Instance
    MIPForest* F1;
    MIPForest* F2;

    // Variables
    IloArray<IloIntVarArray> D;
    IloNumVarArray primalHeuristicVars;
    IloNumArray primalHeuristicVals;

    // CPLEX
    IloEnv env;
    IloCplex solver;
    IloModel model;

    // Model
    void initializeVariableFor(MIPForest* F);
    void setPathConstraintsFor(MIPForest* A, MIPForest* B);

    // CPLEX helps
    void addConstraint(IloNum lhs, IloExpr& expr, IloNum rhs, std::string name);

public:
    MIPModel(): F1(nullptr), F2(nullptr) {};
    MIPModel(MIPForest* F1, MIPForest* F2);
    ~MIPModel();
    MIPModel(const MIPModel&) = delete;
    MIPModel& operator=(const MIPModel&) = delete;
    MIPModel(MIPModel&&) = delete;
    MIPModel& operator=(MIPModel&&) = delete;

    void generateVariables();
    void setPathConstraints();
    void setLowLeafConstraints();
    void setDisconnectedLeafConstraint();
    void setObjective();
    void solve(bool exportModel = false);
    void addPrimalHeuristic(const std::unordered_set<int>& edgesF1, const std::unordered_set<int>& edgesF2);
    int getValueFor(int forestId, int edgeId) const;
    void exportSolution() const;
};



#endif
