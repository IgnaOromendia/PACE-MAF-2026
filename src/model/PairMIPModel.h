#ifndef PAIRMIPMODEL_H
#define PAIRMIPMODEL_H

#include "LazyMIPModel.h"

class UserCutCallback;

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
    void setIncompatiblePathsConstraints();
    void setAgreementPathLeafConstraint();

    // Paths
    void addInitialPathConstraints();
    
    // Constraint Helpers
    void addConflictiveTripleConstraint(const Triple& t);
    void addIncompatiblePathConstraint(const Path& p);
    
public:
    PairMIPModel(): LazyMIPModel() {};
    PairMIPModel(MIPForest* F1, MIPForest* F2): LazyMIPModel(F1, F2) {};
    ~PairMIPModel() override;
    PairMIPModel(const PairMIPModel&) = delete;
    PairMIPModel& operator=(const PairMIPModel&) = delete;
    PairMIPModel(PairMIPModel&&) = delete;
    PairMIPModel& operator=(PairMIPModel&&) = delete;

    int getValueFor(int forestId, int i, int j) const override;
    IloIntVar getVarFor(int forestId, int i, int j) const;

    void generateVariables() override;
    void setConstraints() override;
    void setObjective() override;
    void solve(bool exportModel = false) override;

    // User cut
    void searchForFractionalIncompatiblePaths(UserCutCallback* callback, std::priority_queue<Path>& constraintToAdd, int maxCuts);
    double getCallbackDoubleValueFor(UserCutCallback* callback, int forestId, int i, int j) const;
    void erasePath(const Path& p);


    // Lazy cut
    bool searchForConflictiveTriples(const LazyCallbackI& callback, std::vector<Triple>& constraintToAdd) override;
    bool searchForIncompatiblePaths(const LazyCallbackI& callback, std::vector<Path>& constraintToAdd) override;
    int getCallbackValueFor(const LazyCallbackI& callback, int forestId, int i, int j) const override;
    
};


#endif