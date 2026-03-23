#include "PairMIPModel.h"

PairMIPModel::PairMIPModel(): MIPModel() {
}

PairMIPModel::PairMIPModel(MIPForest *F1, MIPForest *F2): MIPModel(F1, F2) {
    forestNodeAmount.assign(2,0);
    forestNodeAmount[0] = F1->amountOfNodes();
    forestNodeAmount[1] = F2->amountOfNodes();
}

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
    for(int i = 0; i < F1->labelAmount(); i++) {
        for(int j = i + 1; j < F1->labelAmount(); j++) {
            if (not F1->sameConnectedComponent(i,j)) continue;
            for(int k = j + 1; k < F1->labelAmount(); k++) {
                if (not F1->isConflictive(Triple(i,j,k), F2)) continue;

                IloExpr expr(env);
                std::string name = "Tri_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);

                auto [l1, l2] = F1->low(Triple(i,j,k));

                int v = i == l1 or i == l2 ? (j == l1 or j == l2 ? k : j) : i;

                auto [a1, b1] = std::minmax(v, l1);
                auto [a2, b2] = std::minmax(v, l2);

                expr = M[F1->modId()][a1][b1] + M[F1->modId()][a2][b2];

                addConstraint(-IloInfinity, expr, 1, name);
                expr.end();
            }
        }
    }
}

void PairMIPModel::setKnownCutsConstraints() {
    for(int i = 0; i < F1->amountOfNodes(); i++) {
        if (not F1->nodeAvailable(i)) continue;
        for(int j = i+1; j < F1->amountOfNodes(); j++) {
            if (not F1->sameConnectedComponent(i,j) or not F1->nodeAvailable(j)) continue;

            IloExpr expr1(env);
            IloExpr expr2(env);
            
            std::string name1 = "Disc_0" + std::to_string(i) + "_" + std::to_string(j);
            std::string name2 = "Disc_1" + std::to_string(i) + "_" + std::to_string(j);

            expr1 = M[F1->modId()][i][j];
            expr2 = M[F2->modId()][i][j];

            addConstraint(0, expr1, 0, name1);
            addConstraint(0, expr2, 0, name2);

            expr1.end();
            expr2.end();
        }
    }
}

void PairMIPModel::setIncompatiblePaths() {
    for(int i = 0; i < F1->labelAmount(); i++) {
        for(int j = i+1; j < F1->labelAmount(); j++) {
            if (not F1->sameConnectedComponent(i,j)) continue;
            for(int k = j+1; k < F1->labelAmount(); k++) {
                if (not F1->sameConnectedComponent(i,k)) continue;
                for(int l = k+1; l < F1->labelAmount(); l++) {
                    if (not F1->sameConnectedComponent(k,l)) continue;

                    addPermutationIncPath(i,j,k,l);
                    addPermutationIncPath(i,k,j,l);
                    addPermutationIncPath(i,l,j,k);
                }
            }
        }
    }
}

void PairMIPModel::addPermutationIncPath(int a, int b, int c, int d) {
    if (not F1->pathIntersection(a,b,c,d) and F2->pathIntersection(a,b,c,d)) {
        IloExpr expr(env);
        std::string name = "Inc_" + std::to_string(a) + "_" + std::to_string(b) + "_" + std::to_string(c) + "_" + std::to_string(d);
        expr = M[0][a][b] + M[0][c][d];
        addConstraint(-IloInfinity, expr, 1, name);
        expr.end();
    }

    if (F1->pathIntersection(a,b,c,d) and not F2->pathIntersection(a,b,c,d)) {
        IloExpr expr(env);
        std::string name = "Inc_" + std::to_string(a) + "_" + std::to_string(b) + "_" + std::to_string(c) + "_" + std::to_string(d);
        expr = M[1][a][b] + M[1][c][d];
        addConstraint(-IloInfinity, expr, 1, name);
        expr.end();
    }
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

    setConflictiveTripleConstraint();

    setIncompatiblePaths();

    // setKnownCutsConstraints();
}

void PairMIPModel::setObjective() {
    IloExpr obj(env);

    for(int e = 0; e < F1->amountOfEdges(); e++) 
        obj += this->D[F1->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();
}
