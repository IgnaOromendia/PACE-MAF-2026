#ifndef PAIRMIPMODEL_H
#define PAIRMIPMODEL_H

#include "LazyMIPModel.h"

class PairMIPModel: public LazyMIPModel {
private:

    std::vector<int> forestNodeAmount;

    // Variables
    IloArray<IloArray<IloIntVarArray> > M;
    void initializeMVariableFor(MIPForest* F);

    // Constraints
    void setReflexiveConstriantFor(MIPForest* F);
    void setConnectivtyFor(MIPForest* F);
    void setCutPropagationFor(MIPForest* F);
    void setCutConsistencyFor(MIPForest* F);
    void setConflictiveTripleConstraint();
    void setKnownCutsConstraints();
    void setIncompatiblePaths();
    
    // Constraint Helpers
    void addPermutationIncPath(int a, int b, int c, int d);
    void addConflictiveTripleConstraint(int i, int j, int k);
    void addIncompatiblePathConstraint(int t, int i, int j, int k, int l);
    
public:
    PairMIPModel(): LazyMIPModel() {};
    PairMIPModel(MIPForest* F1, MIPForest* F2): LazyMIPModel(F1, F2) {};
    ~PairMIPModel() override;
    PairMIPModel(const PairMIPModel&) = delete;
    PairMIPModel& operator=(const PairMIPModel&) = delete;
    PairMIPModel(PairMIPModel&&) = delete;
    PairMIPModel& operator=(PairMIPModel&&) = delete;

    void generateVariables() override;
    void setConstraints() override;
    void setBasicConstraints() override;
    void setObjective() override;
    void solve(bool exportModel = false) override;

    // Lazy
    bool searchForIncompatiblePaths(MIPForest* A, MIPForest* B) override;
    
    int getValueFor(int forestId, int i, int j) const override;
};


#endif