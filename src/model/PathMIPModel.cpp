#include "PathMIPModel.h"

void PathMIPModel::setPathConstraintsFor(MIPForest* A, MIPForest* B) {
    for(int v = 0; v < A->amountOfLabels(); v++) {
        for(int w = v + 1; w < A->amountOfLabels(); w++) {
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

void PathMIPModel::setConstraints() {
    setPathConstraints();
    setLowLeafConstraints();
    setDisconnectedLeafConstraint();
}

void PathMIPModel::setPathConstraints() {
    setPathConstraintsFor(F1, F2);
    setPathConstraintsFor(F2, F1);
}

void PathMIPModel::setLowLeafConstraints() {
    std::vector<char> used(F1->amountOfEdges(), 0);
    int count = 1;

    for(int v = 0; v < F1->amountOfLabels(); v++) {
        for(int w = v + 1; w < F1->amountOfLabels(); w++) {
            if (not F1->sameConnectedComponent(v,w)) continue;
            for(int z = w + 1; z < F1->amountOfLabels(); z++) {
                if (not F1->sameConnectedComponent(w, z)) continue;

                Triple t = Triple(v,w,z);
                
                if (not F1->isConflictive(t, F2)) continue;

                auto [j, k] = F1->low(t);
                int i = v == j or v == k ? (w == j or w == k ? z : w) : v;

                addConflictiveTripleConstraint(Triple(i,j,k), used, count++);
            }
        }
    }
}

void PathMIPModel::setDisconnectedLeafConstraint() {
    for(int v = 0; v < F1->amountOfLabels(); v++) {
        for(int w = v + 1; w < F1->amountOfLabels(); w++) {
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

void PathMIPModel::addConflictiveTripleConstraint(const Triple &t, std::vector<char>& used, int count) {
    IloExpr expr(env);
    std::string name = "Q_" + std::to_string(t.i) + "_" + std::to_string(t.j) + "_" + std::to_string(t.k);

    for(int edgeId : F1->pathBetween(t.i,t.j)) {
        expr += this->D[F1->modId()][edgeId];
        used[edgeId] = count;
    }

    for(int edgeId : F1->pathBetween(t.i,t.k)) 
        if (used[edgeId] < count)
            expr += this->D[F1->modId()][edgeId];

    addConstraint(1, expr, IloInfinity, name);
    expr.end();
}

void PathMIPModel::setObjective() {
    IloExpr obj(env);

    for(int e = 0; e < F1->amountOfEdges(); e++) 
        obj += this->D[F1->modId()][e];
    
    model.add(IloMinimize(env, obj));
    obj.end();
}
