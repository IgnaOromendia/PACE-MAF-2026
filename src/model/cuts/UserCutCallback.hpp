#ifndef USER_CUT_CALLBACK_HPP
#define USER_CUT_CALLBACK_HPP

#include "../PairMIPModel.h"

class UserCutCallback: public IloCplex::UserCutCallbackI {
private:
    PairMIPModel* mip;
    const double eps = 1e-6;
    int maxCutPerCall;
public:
    UserCutCallback(IloEnv env, PairMIPModel* mip, int maxCuts = 20): IloCplex::UserCutCallbackI(env), mip(mip), maxCutPerCall(maxCuts) {};

    void main() {
        if (not isAfterCutLoop()) return;

        std::vector<Path> violatedPaths; 

        mip->searchForFractionalIncompatiblePaths(this, violatedPaths, maxCutPerCall, eps);

        if (violatedPaths.empty()) return;

        IloEnv env = getEnv();

        for (const Path& p: violatedPaths) {
            IloExpr expr(env);
            expr = mip->getVarFor(p.tree, p.i, p.j) + mip->getVarFor(p.tree, p.k, p.l);
            add(expr <= 1.0).end();
            expr.end();
        }
    }

    IloCplex::CallbackI* duplicateCallback() const {
        return new (getEnv()) UserCutCallback(*this);
    }
};

#endif