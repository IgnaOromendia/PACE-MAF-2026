#include "MIPSolver.h"

MIPSolver::MIPSolver() {
    env = IloEnv();
}

MIPSolver::~MIPSolver() {
    env.end();
}

MIPForest* MIPSolver::solve(Instance instance) {
    forests = instance.mipTrees();

    size_t index = 1;

    MIPForest* F = forests.front();
    
    while(index < forests.size()) 
        F = solveFor(F, forests[index++]);
    
    return F;
}

MIPForest* MIPSolver::solveFor(MIPForest* F1, MIPForest* F2) {
    // Inicializa modelo
    clearModel();

    // MIP generation
    generateVariablesWith(F1, F2);
    setPathConstraints(F1, F2);
    setPathConstraints(F2, F1);
    setLowLeafConstraints(F1, F2);
    setObjective(F1);

    // Solve
    solve();
    // exportSolution();

    MIPForest* MAF = pruneAndRegraft(F1);

    return MAF;
}

void MIPSolver::generateVariablesWith(MIPForest* F1, MIPForest* F2) {
    this->D = IloArray<IloIntVarArray>(env, 2);
    initializeVariableFor(F1);
    initializeVariableFor(F2);    
}

void MIPSolver::initializeVariableFor(MIPForest* F) {
    D[F->modId()] = IloIntVarArray(env, F->amountOfEdges(), 0, 1);

    for(int i = 0; i < F->amountOfEdges(); i++) {
        std::string name = "D_" + std::to_string(F->modId()) + "_" + std::to_string(i);
        D[F->modId()][i].setName(name.c_str());
    }

    model.add(D[F->modId()]);
}

void MIPSolver::setPathConstraints(MIPForest* F1, MIPForest* F2) {
    for(int v = 0; v < F1->labelAmount(); v++) {
        for(int w = v + 1; w < F1->labelAmount(); w++) {
            IloExpr expr(env);
            std::string name = "Path_" + std::to_string(F1->modId()) + "_" + std::to_string(v) + "_" + std::to_string(w);

            for(int edgeId : F1->pathBetween(v,w))
                expr += this->D[F1->modId()][edgeId];

            int pathSize = F1->pathSize(v,w);

            for(int edgeId : F2->pathBetween(v,w))
                expr -= pathSize * this->D[F2->modId()][edgeId];

            addConstraint(-IloInfinity, expr, 0, name);
            expr.end();
        }
    }
}

void MIPSolver::setLowLeafConstraints(MIPForest* F1, MIPForest* F2) {
    std::vector<char> used(F1->amountOfEdges(), 0);
    int count = 1;

    for(int v = 0; v < F1->labelAmount(); v++) {
        for(int w = v + 1; w < F1->labelAmount(); w++) {
            for(int z = w + 1; z < F1->labelAmount(); z++) {
                if (not isConflictedLeaf(Triple(v,w,z), F1, F2)) continue;

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

void MIPSolver::addConstraint(IloNum lhs, IloExpr& expr, IloNum rhs, std::string name) {
    model.add(IloRange(env, lhs, expr, rhs, name.c_str()));
}

void MIPSolver::setObjective(MIPForest* F) {
    IloExpr obj(env);

    for(int e = 0; e < F->amountOfEdges(); e++) 
        obj += this->D[F->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();

}

void MIPSolver::solve() {
    solver.extract(model);
    solver.exportModel(equationsFile.c_str());
    solver.solve();

    double objValue = solver.getBestObjValue();
}

void MIPSolver::exportSolution() const {
    int status = solver.getStatus();

    if (status == 3) return;

    for(MIPForest* F : forests) {
        std::cout << "F" << F->id() << std::endl;
        for(int e = 0; e < F->amountOfEdges(); e++) {
            double value = solver.getValue(this->D[F->modId()][e]);
            std::cout << e << ": " << value << std::endl;
        }
    }
}

bool MIPSolver::isConflictedLeaf(Triple t, MIPForest* F1,  MIPForest* F2) const {
    return F1->low(t) != F2->low(t);
}

MIPForest* MIPSolver::pruneAndRegraft(MIPForest* F) const {
    MIPForest* newForest = new MIPForest(*F);

    // Cutting and Regraft
    for(int e = 0; e < newForest->amountOfEdges(); e++) {
        int cutting = solver.getValue(this->D[newForest->modId()][e]);

        if (not cutting) continue;

        auto [descendant, ancestor] = newForest->nodesOf(e);

        newForest->cut(descendant);
    }

    newForest->regraft();

    return newForest;
}

void MIPSolver::clearModel() {
    if (D.getImpl()) D.end();
    if (model.getImpl()) model.end();
    if (solver.getImpl()) solver.end();

    model = IloModel(env);
    solver = IloCplex(env);
    solver.setOut(env.getNullStream());
}

