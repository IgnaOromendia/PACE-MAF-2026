#include "PairMIPModel.h"
#include "cuts/LazyCallbackI.hpp"
#include "cuts/UserCutCallback.hpp"

PairMIPModel::~PairMIPModel() {
    if (M.getImpl()) M.end();
}

void PairMIPModel::initializeMVariableFor(MIPForest *F) {
    M[F->modId()] = IloArray<IloIntVarArray>(env, F->amountOfNodes());

    for(int i = 0; i < F->amountOfNodes(); i++) {
        M[F->modId()][i] = IloIntVarArray(env, F->amountOfNodes(), 0, 1);

        for(int j = 0; j < F->amountOfNodes(); j++) {
            std::string name = "M_" + std::to_string(F->modId()) + "_" + std::to_string(i)  + "_" + std::to_string(j);
            M[F->modId()][i][j].setName(name.c_str());
        }

        model.add(M[F->modId()][i]);
    }    
}

void PairMIPModel::setReflexiveConstriantFor(MIPForest *F) {
    for(int i = 0; i < F->amountOfNodes(); i++) {
        if (not F->nodeAvailable(i)) continue;
        IloExpr expr(env);
        std::string name = "Reflex_" + std::to_string(F->modId()) + "_" + std::to_string(i);
        expr = M[F->modId()][i][i];
        addConstraint(1, expr, 1, name);
        expr.end();  
    }
}

void PairMIPModel::setConnectivtyFor(MIPForest *F) {
    for(int i = 0; i < F->amountOfNodes(); i++) {
        if (not F->nodeAvailable(i)) continue;
        for(int j = i + 1; j < F->amountOfNodes(); j++) {
            if (not F->sameConnectedComponent(i,j) or not F->nodeAvailable(j)) continue;

            IloExpr expr(env);
            std::string name = "ConnProp_" + std::to_string(F->modId()) + "_" + std::to_string(i) + "_" + std::to_string(j);
            int v = F->nextNodeInPathTo(i,j);

            expr = M[F->modId()][i][j] - M[F->modId()][std::min(v,j)][std::max(v,j)];

            addConstraint(-IloInfinity, expr, 0, name);
            expr.end();
        }
    }
}

void PairMIPModel::setCutPropagationFor(MIPForest* F) {
    for(int i = 0; i < F->amountOfNodes(); i++) {
        if (not F->nodeAvailable(i)) continue;
        for(int j = i + 1; j < F->amountOfNodes(); j++) {
            if (not F->sameConnectedComponent(i,j) or not F->nodeAvailable(j)) continue;

            IloExpr expr(env);
            std::string name = "CutProp_" + std::to_string(F->modId()) + "_" + std::to_string(i) + "_" + std::to_string(j);
            
            int v = F->nextNodeInPathTo(i,j);
            int e = F->edgeForNode(i, v);
            
            expr = M[F->modId()][i][j] - M[F->modId()][std::min(v,j)][std::max(v,j)] + D[F->modId()][e];

            addConstraint(0, expr, IloInfinity, name);
            expr.end();
        }
    }
}

void PairMIPModel::setCutConsistencyFor(MIPForest* F) {
    for(int i = 0; i < F->amountOfNodes(); i++) {
        if (not F->nodeAvailable(i)) continue;
        for(int j = i+1; j < F->amountOfNodes(); j++) {
            if (not F->sameConnectedComponent(i,j) or not F->nodeAvailable(j)) continue;

            IloExpr expr(env);
            std::string name = "CutCons_" + std::to_string(F->modId()) + "_" + std::to_string(i) + "_" + std::to_string(j);
            
            int v = F->nextNodeInPathTo(i,j);
            int e = F->edgeForNode(i,v);

            expr = M[F->modId()][i][j] + D[F->modId()][e];

            addConstraint(-IloInfinity, expr, 1, name);
            expr.end();
        }
    }
}

void PairMIPModel::setConflictiveTripleConstraint() {
    for(const Triple& t: conflictiveTriples) 
        addConflictiveTripleConstraint(t);
}

void PairMIPModel::setKnownCutsConstraints() {
    for(int i = 0; i < F1->labelAmount(); i++) {
        for(int j = i+1; j < F1->labelAmount(); j++) {
            if (F1->sameConnectedComponent(i,j)) continue;

            // IloExpr expr1(env);
            IloExpr expr2(env);
            
            // std::string name1 = "Disc_0" + std::to_string(i) + "_" + std::to_string(j);
            std::string name2 = "Disc_1" + std::to_string(i) + "_" + std::to_string(j);

            // expr1 = M[F1->modId()][i][j];
            expr2 = M[F2->modId()][i][j];

            // addConstraint(0, expr1, 0, name1);
            addConstraint(0, expr2, 0, name2);

            // expr1.end();
            expr2.end();
        }
    }
}

void PairMIPModel::setIncompatiblePathsConstraints() {
    for(const Path& p: incompatiblePaths) 
        addIncompatiblePathConstraint(p);
    
}

void PairMIPModel::setAgreementPathLeafConstraint() {
    for(int i = 0; i < F1->labelAmount(); i++) {
        for(int j = i + 1; j < F1->labelAmount(); j++) {
            if (not F1->sameConnectedComponent(i,j)) continue;
            IloExpr expr(env);
            std::string name = "SamePath_" + std::to_string(i) + "_" + std::to_string(j);

            expr = M[F1->modId()][i][j] - M[F2->modId()][i][j];

            addConstraint(0, expr, 0, name);
            expr.end();
        }
    }
}

void PairMIPModel::addConflictiveTripleConstraint(const Triple& t) {
    IloExpr expr(env);
    std::string name = "Tri_" + std::to_string(t.i) + "_" + std::to_string(t.j) + "_" + std::to_string(t.k);

    auto [a1, b1] = std::minmax(t.i, t.j);
    auto [a2, b2] = std::minmax(t.i, t.k);

    expr = M[F1->modId()][a1][b1] + M[F1->modId()][a2][b2];

    addConstraint(-IloInfinity, expr, 1, name);
    expr.end();
}

void PairMIPModel::addIncompatiblePathConstraint(const Path& p) {
    IloExpr expr(env);
    std::string name = "Inc_" + std::to_string(p.tree) + "_" + std::to_string(p.i) + "_" + std::to_string(p.j) + "_" + std::to_string(p.k) + "_" + std::to_string(p.l);
    expr = M[p.tree][p.i][p.j] + M[p.tree][p.k][p.l];
    addConstraint(-IloInfinity, expr, 1, name);
    expr.end();
}

int PairMIPModel::getValueFor(int forestId, int i, int j) const {
    try {
        const double value = solver.getValue(M[forestId][std::min(i,j)][std::max(i,j)]);
        return value >= 0.5 ? 1 : 0;
    } catch (const IloException& e) {
        std::cerr << "CPLEX exception in getMValueFor(" << forestId << ", " << std::min(i,j) << ", " << std::max(i,j) << "): " << e << "\n";
        throw;
    }
}

IloIntVar PairMIPModel::getVarFor(int forestId, int i, int j) const {
    return M[forestId][std::min(i,j)][std::max(i,j)];
}

void PairMIPModel::generateVariables() {
    M = IloArray<IloArray<IloIntVarArray>>(env, 2);
    D = IloArray<IloIntVarArray>(env, 2);
    initializeMVariableFor(F1);
    initializeMVariableFor(F2);
    initializeDVariableFor(F1);
    initializeDVariableFor(F2);
}

void PairMIPModel::setConstraints() {
    setReflexiveConstriantFor(F1);
    setReflexiveConstriantFor(F2);

    setConnectivtyFor(F1);
    setConnectivtyFor(F2);

    setCutPropagationFor(F1);
    setCutPropagationFor(F2);

    setCutConsistencyFor(F1);
    setCutConsistencyFor(F2);

    setAgreementPathLeafConstraint();

    setConflictiveTripleConstraint();

    // setIncompatiblePathsConstraints();

    setKnownCutsConstraints();
}

void PairMIPModel::setObjective() {
    IloExpr obj(env);

    for(int e = 0; e < F1->amountOfEdges(); e++) 
        obj += this->D[F1->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();
}

void PairMIPModel::solve(bool exportModel) {
    // solver.use(LazyCallback(env, this));
    solver.use(new (env) UserCutCallback(env, this));
    cplexSolve(exportModel);
}

void PairMIPModel::searchForFractionalIncompatiblePaths(UserCutCallback* callback, std::vector<Path> &constraintToAdd, int maxCuts, double eps) {
    constraintToAdd.reserve(maxCuts);

    for(auto it = incompatiblePaths.begin(); it != incompatiblePaths.end() and constraintToAdd.size() < maxCuts;) {
        if (constraintToAdd.size() == pathsBound) break;

        double lhs = getCallbackDoubleValueFor(callback, (*it).tree, (*it).i, (*it).j) + getCallbackDoubleValueFor(callback, (*it).tree, (*it).k, (*it).l);

        if (lhs <= 1 + eps) {
            it++;
            continue;
        }

        constraintToAdd.push_back(*it);
        it = incompatiblePaths.erase(it);
    }
    
}

double PairMIPModel::getCallbackDoubleValueFor(UserCutCallback* callback, int forestId, int i, int j) const {
    return callback->getValue(M[forestId][std::min(i,j)][std::max(i,j)]);;
}

bool PairMIPModel::searchForConflictiveTriples(const LazyCallbackI& callback, std::vector<Triple>& constraintToAdd) {    
    constraintToAdd.reserve(triplesBound);

    for(auto it = conflictiveTriples.begin(); it != conflictiveTriples.end();) {
        if (constraintToAdd.size() == triplesBound) break;

        int lhs = getCallbackValueFor(callback, F1->modId(), (*it).i, (*it).j) + getCallbackValueFor(callback, F1->modId(), (*it).i, (*it).k);

        if (lhs <= 1) {
            it++;
            continue;
        }

        constraintToAdd.push_back(*it);
        it = conflictiveTriples.erase(it);
    }

    if (constraintToAdd.size() >= size_t(triplesBound * 0.8)) triplesBound *= 3;     

    return not constraintToAdd.empty();
}

bool PairMIPModel::searchForIncompatiblePaths(const LazyCallbackI& callback, std::vector<Path>& constraintToAdd) {
    constraintToAdd.reserve(pathsBound);

    for(auto it = incompatiblePaths.begin(); it != incompatiblePaths.end();) {
        if (constraintToAdd.size() == pathsBound) break;

        int lhs = getCallbackValueFor(callback, (*it).tree, (*it).i, (*it).j) + getCallbackValueFor(callback, (*it).tree, (*it).k, (*it).l);

        if (lhs <= 1) {
            it++;
            continue;
        }

        constraintToAdd.push_back(*it);
        it = incompatiblePaths.erase(it);
    }

    if (constraintToAdd.size() >= size_t(pathsBound * 0.8)) pathsBound *= 3; 

    return not constraintToAdd.empty();
}

int PairMIPModel::getCallbackValueFor(const LazyCallbackI &callback, int forestId, int i, int j) const {
    double value = callback.getValue(M[forestId][std::min(i,j)][std::max(i,j)]);
    return value >= 0.5 ? 1 : 0;
}
