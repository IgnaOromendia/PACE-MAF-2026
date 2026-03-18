#include "MIPModel.h"

MIPModel::MIPModel(MIPForest *F1, MIPForest *F2) {
    this->F1 = F1;
    this->F2 = F2;

    model = IloModel(env);
    solver = IloCplex(env);
    solver.setOut(env.getNullStream());

    primalHeuristicVars = IloNumVarArray(env);
    primalHeuristicVals = IloNumArray(env);
}

MIPModel::~MIPModel() {
    if (D.getImpl()) D.end();
    if (model.getImpl()) model.end();
    if (solver.getImpl()) solver.end();
    if (primalHeuristicVars.getImpl()) primalHeuristicVars.end();
    if (primalHeuristicVals.getImpl()) primalHeuristicVals.end();
    env.end();
}

void MIPModel::generateVariables() {
    D = IloArray<IloIntVarArray>(env, 2);
    initializeVariableFor(F1);
    initializeVariableFor(F2);
}

void MIPModel::setPathConstraints() {
    setPathConstraintsFor(F1, F2);
    setPathConstraintsFor(F2, F1);
}

void MIPModel::setLowLeafConstraints() {
    std::vector<char> used(F1->amountOfEdges(), 0);
    int count = 1;

    for(int v = 0; v < F1->labelAmount(); v++) {
        for(int w = v + 1; w < F1->labelAmount(); w++) {
            if (not F1->sameConnectedComponent(v,w)) continue;
            for(int z = w + 1; z < F1->labelAmount(); z++) {
                // if (not F1->sameConnectedComponent(w,z))
                if (not F1->isConflictive(Triple(v,w,z), F2)) continue;

                IloExpr expr(env);
                std::string name = "Q_" + std::to_string(v) + "_" + std::to_string(w) + "_" + std::to_string(z);

                for(int edgeId : F1->pathBetween(v,w)) {
                    expr += this->D[F1->modId()][edgeId];
                    used[edgeId] = count;
                }
                    

                for(int edgeId : F1->pathBetween(v,z)) 
                    if (used[edgeId] < count)
                        expr += this->D[F1->modId()][edgeId];

                count++;
                addConstraint(1, expr, IloInfinity, name);
                expr.end();
            }
        }
    }
}

void MIPModel::setDisconnectedLeafConstraint() {
    for(int v = 0; v < F1->labelAmount(); v++) {
        for(int w = v + 1; w < F1->labelAmount(); w++) {
            if (F1->sameConnectedComponent(v,w)) continue;

            IloExpr expr(env);
            std::string name = "PathCut_" + std::to_string(F2->modId()) + "_" + std::to_string(v) + "_" + std::to_string(w);

            for(int edgeId : F2->pathBetween(v,w))
                expr += this->D[F2->modId()][edgeId];

            addConstraint(1, expr, IloInfinity, name);
            expr.end();
        }
    }
}

void MIPModel::setObjective() {
    IloExpr obj(env);

    for(int e = 0; e < F1->amountOfEdges(); e++) 
        obj += this->D[F1->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();
}

void MIPModel::solve(bool exportModel) {
    solver.extract(model);
    // solver.addMIPStart(primalHeuristicVars, primalHeuristicVals);
    if (exportModel) {
        equationsFile += std::to_string(F1->id()) + "_" + std::to_string(F2->id()) + ".lp";
        solver.exportModel(equationsFile.c_str());
    }
    try {
        bool ok = solver.solve();
        std::cerr << "solve=" << ok << " status=" << solver.getStatus() << "\n";
    } catch (const IloException& e) {
        std::cerr << "CPLEX exception in solve(): " << e << "\n";
        throw;
    }
}

void MIPModel::addPrimalHeuristic(const std::unordered_set<int> &edgesF1, const std::unordered_set<int> &edgesF2) {

    for(int e = 0; e < D[0].getSize(); e++) {
        primalHeuristicVars.add(D[0][e]);
        primalHeuristicVals.add(edgesF1.count(e) ? 1.0 : 0.0);
    }

    for(int e = 0; e < D[1].getSize(); e++) {
        primalHeuristicVars.add(D[1][e]);
        primalHeuristicVals.add(edgesF2.count(e) ? 1.0 : 0.0);
    }

}

int MIPModel::getValueFor(int forestId, int edgeId) const {
    try {
        return solver.getValue(D[forestId][edgeId]);
    } catch (const IloException& e) {
        std::cerr << "CPLEX exception in getValueFor(" << forestId << ", " << edgeId << "): " << e << "\n";
        throw;
    }
}

void MIPModel::exportSolution() const {
    for(MIPForest* F : {F1, F2}) {
        std::cout << "F" << F->id() << std::endl;
        for(int e = 0; e < F->amountOfEdges(); e++) {
            double value = solver.getValue(this->D[F->modId()][e]);
            std::cout << e << ": " << value << std::endl;
        }
    }
}

void MIPModel::initializeVariableFor(MIPForest* F) {
    D[F->modId()] = IloIntVarArray(env, F->amountOfEdges(), 0, 1);

    for(int i = 0; i < F->amountOfEdges(); i++) {
        std::string name = "D_" + std::to_string(F->modId()) + "_" + std::to_string(i);
        D[F->modId()][i].setName(name.c_str());
    }

    model.add(D[F->modId()]);
}

void MIPModel::setPathConstraintsFor(MIPForest* A, MIPForest* B) {
    for(int v = 0; v < A->labelAmount(); v++) {
        for(int w = v + 1; w < A->labelAmount(); w++) {
            int pathSize = A->pathSize(v,w);
            if (pathSize == 0 or B->pathSize(v,w) == 0) continue;

            IloExpr expr(env);
            std::string name = "Path_" + std::to_string(A->modId()) + "_" + std::to_string(v) + "_" + std::to_string(w);

            for(int edgeId : A->pathBetween(v,w))
                expr += this->D[A->modId()][edgeId];

            for(int edgeId : B->pathBetween(v,w))
                expr -= pathSize * this->D[B->modId()][edgeId];

            addConstraint(-IloInfinity, expr, 0, name);
            expr.end();
        }
    }
}

void MIPModel::addConstraint(IloNum lhs, IloExpr &expr, IloNum rhs, std::string name) {
    model.add(IloRange(env, lhs, expr, rhs, name.c_str()));
}
