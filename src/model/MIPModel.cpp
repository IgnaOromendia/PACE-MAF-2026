#include "MIPModel.h"

MIPModel::MIPModel(MIPForest *F1, MIPForest *F2) {
    this->F1 = F1;
    this->F2 = F2;

    F1->conflictiveTriples(F2, conflictiveTriples);
    F1->incompatiblePaths(F2, incompatiblePaths);

    model = IloModel(env);
    solver = IloCplex(env);

    primalHeuristicVars = IloNumVarArray(env);
    primalHeuristicVals = IloNumArray(env);

    // PARAMS
    // solver.setOut(env.getNullStream());
    solver.setParam(IloCplex::Param::Threads, 1);
    solver.setParam(IloCplex::Param::Emphasis::MIP, IloCplex::MIPEmphasisFeasibility);
    // solver.setParam(IloCplex::Param::MIP::Strategy::Search, IloCplex::Traditional);
    // solver.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, 1); // best bound
    // solver.setParam(IloCplex::Param::MIP::Strategy::VariableSelect, 3); // strong branching
    // solver.setParam(IloCplex::Param::MIP::Cuts::Gomory, 2);
    // solver.setParam(IloCplex::Param::MIP::Cuts::MIRCut, 2);
    // solver.setParam(IloCplex::Param::MIP::Cuts::FlowCovers, 2);
    // solver.setParam(IloCplex::Param::MIP::Cuts::Cliques, 2);
    // solver.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.1);
    // solver.setParam(IloCplex::Param::TimeLimit, 30.0);
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
    initializeDVariableFor(F1);
    initializeDVariableFor(F2);
}

void MIPModel::cplexSolve(bool exportModel) {
    try {
        solver.extract(model);
        if (primalHeuristicVars.getSize() > 0) solver.addMIPStart(primalHeuristicVars, primalHeuristicVals);
        if (exportModel) {
            std::string name = equationsFile + std::to_string(F1->id()) + "_" + std::to_string(F2->id()) + "_run_" + std::to_string(run) + ".lp";
            solver.exportModel(name.c_str());
        }

        run++;

        const auto start = std::chrono::steady_clock::now();
        bool ok = solver.solve();
        const auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = end - start;
        
        std::cerr << "solve=" << ok
                  << " status=" << solver.getStatus()
                  << " solverTime=" << elapsed.count() << "s\n";
    } catch (const IloException& e) {
        std::cerr << "CPLEX exception in solve(): " << e << "\n";
        throw;
    }
}

void MIPModel::addPrimalHeuristic(const std::unordered_set<int> &edgesToCut) {

    for (int e: edgesToCut) {
        primalHeuristicVars.add(D[0][e]);
        primalHeuristicVals.add(1.0);
    }
    
}

int MIPModel::getValueFor(int forestId, int edgeId) const {
    try {
        const double value = solver.getValue(D[forestId][edgeId]);
        return value >= 0.5 ? 1 : 0;
    } catch (const IloException& e) {
        std::cerr << "CPLEX exception in getValueFor(" << forestId << ", " << edgeId << "): " << e << "\n";
        throw;
    }
}

int MIPModel::getValueFor(int forestId, int i, int j) const {
    int edgeId = forestId % 2 == 0 ? F1->edgeForNode(i,j) : F2->edgeForNode(i,j);
    try {
        const double value = solver.getValue(D[forestId][edgeId]);
        return value >= 0.5 ? 1 : 0;
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

bool MIPModel::isInfeasible() const {
    return solver.getStatus() == 3;
}

void MIPModel::initializeDVariableFor(MIPForest* F) {
    D[F->modId()] = IloIntVarArray(env, F->amountOfEdges(), 0, 1);

    for(int i = 0; i < F->amountOfEdges(); i++) {
        std::string name = "D_" + std::to_string(F->modId()) + "_" + std::to_string(i);
        D[F->modId()][i].setName(name.c_str());
    }

    model.add(D[F->modId()]);
}

void MIPModel::addConstraint(IloNum lhs, IloExpr &expr, IloNum rhs, std::string name) {
    model.add(IloRange(env, lhs, expr, rhs, name.c_str()));
}
