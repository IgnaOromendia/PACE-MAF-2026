#ifndef PAIRMIPMODEL_H
#define PAIRMIPMODEL_H

#include "MIPModel.h"

class PairMIPModel: public MIPModel {
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
    void addPermutationIncPath(int a, int b, int c, int d);
    
public:
    PairMIPModel();
    PairMIPModel(MIPForest* F1, MIPForest* F2);
    ~PairMIPModel() override;
    PairMIPModel(const PairMIPModel&) = delete;
    PairMIPModel& operator=(const PairMIPModel&) = delete;
    PairMIPModel(PairMIPModel&&) = delete;
    PairMIPModel& operator=(PairMIPModel&&) = delete;

    void generateVariables() override;
    void setConstraints() override;
    void setObjective() override;
};


#endif