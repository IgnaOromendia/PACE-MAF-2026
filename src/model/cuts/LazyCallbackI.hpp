#ifndef LAZY_CALLBACK_I_HPP
#define LAZY_CALLBACK_I_HPP

#include "PairMIPModel.h"

class LazyCallbackI: public IloCplex::LazyConstraintCallbackI {
private:
    PairMIPModel* mip;
public:
    LazyCallbackI(IloEnv env, PairMIPModel* mip): IloCplex::LazyConstraintCallbackI(env), mip(mip) {}
    ~LazyCallbackI() override = default;

    void main() override {
        std::vector<Triple> violatedTriples;
        std::vector<Path> violatedPaths;

        mip->searchForConflictiveTriples(*this, violatedTriples);
        mip->searchForIncompatiblePaths(*this, violatedPaths);

        for (const Triple& t : violatedTriples) {
            IloExpr expr(getEnv());
            expr = mip->getVarFor(0, t.i, t.j) + mip->getVarFor(0, t.i, t.k);
            add(expr <= 1);
            expr.end();
        }

        for (const Path& p : violatedPaths) {
            IloExpr expr(getEnv());
            expr = mip->getVarFor(p.tree, p.i, p.j) + mip->getVarFor(p.tree, p.k, p.l);
            add(expr <= 1);
            expr.end();
        }
    }

    IloCplex::CallbackI* duplicateCallback() const override {
        return new (getEnv()) LazyCallbackI(*this);
    }

};

inline IloCplex::Callback LazyCallback(IloEnv env, PairMIPModel* mip) {
    return IloCplex::Callback(new (env) LazyCallbackI(env, mip));
}




#endif
