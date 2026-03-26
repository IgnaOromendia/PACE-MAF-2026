#include "PairMIPModel.h"

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

                auto [l1, l2] = F1->low(Triple(i,j,k));
                int v = i == l1 or i == l2 ? (j == l1 or j == l2 ? k : j) : i;
                addConflictiveTripleConstraint(v, l1, l2);
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
    if (not F1->pathIntersection(a,b,c,d) and F2->pathIntersection(a,b,c,d)) 
        addIncompatiblePathConstraint(F1->modId(), a, b, c, d);
    
    if (F1->pathIntersection(a,b,c,d) and not F2->pathIntersection(a,b,c,d)) 
        addIncompatiblePathConstraint(F2->modId(), a, b, c, d);
}

void PairMIPModel::addConflictiveTripleConstraint(int i, int j, int k) {
    IloExpr expr(env);
    std::string name = "Tri_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);

    auto [a1, b1] = std::minmax(i, j);
    auto [a2, b2] = std::minmax(i, k);

    expr = M[F1->modId()][a1][b1] + M[F1->modId()][a2][b2];

    addConstraint(-IloInfinity, expr, 1, name);
    expr.end();
}

void PairMIPModel::addIncompatiblePathConstraint(int t, int i, int j, int k, int l) {
    IloExpr expr(env);
    std::string name = "Inc_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k) + "_" + std::to_string(l);
    expr = M[t][i][j] + M[t][k][l];
    addConstraint(-IloInfinity, expr, 1, name);
    expr.end();
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

void PairMIPModel::setBasicConstraints() {
    setReflexiveConstriantFor(F1);
    setReflexiveConstriantFor(F2);

    setConnectivtyFor(F1);
    setConnectivtyFor(F2);

    setCutPropagationFor(F1);
    setCutPropagationFor(F2);

    setCutConsistencyFor(F1);
    setCutConsistencyFor(F2);

    setConflictiveTripleConstraint();
}

void PairMIPModel::setObjective() {
    IloExpr obj(env);

    for(int e = 0; e < F1->amountOfEdges(); e++) 
        obj += this->D[F1->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();
}

void PairMIPModel::solve(bool exportModel) {
    while (true) {
        cplexSolve(exportModel);
        // mip->exportSolution();

        if (isInfeasible()) {
            std::cerr << "Infeasible\n";
            break;
        }

        // const auto start = std::chrono::steady_clock::now();
        if (searchForIncompatiblePaths(F1, F2)) {
            // const auto end = std::chrono::steady_clock::now();
            // const std::chrono::duration<double> elapsed = end - start;
            // std::cerr << "lazy triples time: " << elapsed.count() << "\n"; 
            continue;
        }
        
        break;
    }    
}

bool PairMIPModel::searchForIncompatiblePaths(MIPForest* A, MIPForest* B) {
    int L = A->labelAmount();
    std::vector<Path> violatedPaths;
    violatedPaths.reserve(constriantBound);

    // std::cout << constraintAddAmount << "\n";

    for(int i = 0; i < L and violatedPaths.size() < constriantBound; i++) {
        for(int j = i+1; j < L and violatedPaths.size() < constriantBound; j++) {
            if (not A->sameConnectedComponent(i,j)) continue;
            for(int k = j+1; k < L and violatedPaths.size() < constriantBound; k++) {
                if (not A->sameConnectedComponent(i,k)) continue;
                for(int l = k+1; l < L and violatedPaths.size() < constriantBound; l++) {
                    if (not A->sameConnectedComponent(k,l)) continue;

                    if (not F1->pathIntersection(i,j,k,l) and F2->pathIntersection(i,j,k,l)) {
                        int constraintValue = getValueFor(A->modId(), i, j) + getValueFor(A->modId(), k, l);
                        if (constraintValue > 1) 
                            violatedPaths.emplace_back(A->modId(), i, j, k, l);
                    }
                    
                    if (F1->pathIntersection(i,j,k,l) and not F2->pathIntersection(i,j,k,l)) {
                        int constraintValue = getValueFor(B->modId(), i, j) + getValueFor(B->modId(), k, l);
                        if (constraintValue > 1) 
                            violatedPaths.emplace_back(B->modId(), i, j, k, l);
                    }
                    
                }
            }
        }
    }

    constriantBound *= 3; 

    for (const Path& p : violatedPaths)
        addIncompatiblePathConstraint(p.tree, p.i, p.j, p.k, p.l);

    return not violatedPaths.empty();
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
