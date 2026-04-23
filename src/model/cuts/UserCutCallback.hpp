#ifndef USER_CUT_CALLBACK_HPP
#define USER_CUT_CALLBACK_HPP

#include "../PairMIPModel.h"

class UserCutCallback: public IloCplex::UserCutCallbackI {
private:
    PairMIPModel* mip;
    int maxCutPerCall;
public:
    UserCutCallback(IloEnv env, PairMIPModel* mip, int maxCuts = 20): IloCplex::UserCutCallbackI(env), mip(mip), maxCutPerCall(maxCuts) {};

    void main() {
        if (not isAfterCutLoop()) return;
        if (getNnodes() % 20 != 0) return;


        std::priority_queue<Path> violatedPaths; 

        mip->searchForFractionalIncompatiblePaths(this, violatedPaths, maxCutPerCall);

        IloEnv env = getEnv();

        while(not violatedPaths.empty()) {
            IloExpr expr(env);
            
            const Path& p = violatedPaths.top();
            violatedPaths.pop();

            mip->erasePath(p);

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