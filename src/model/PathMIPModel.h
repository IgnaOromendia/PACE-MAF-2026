#ifndef PATHMIPMODEL_H
#define PATHMIPMODEL_H

#include "MIPModel.h"

class PathMIPModel: public MIPModel {
private:

    void setPathConstraintsFor(MIPForest* A, MIPForest* B);

    // Constraints
    void setPathConstraints();
    void setLowLeafConstraints();
    void setDisconnectedLeafConstraint();
    
public:
    PathMIPModel(): MIPModel() {};
    PathMIPModel(MIPForest* F1, MIPForest* F2): MIPModel(F1, F2) {};
    ~PathMIPModel() override {};
    PathMIPModel(const PathMIPModel&) = delete;
    PathMIPModel& operator=(const PathMIPModel&) = delete;
    PathMIPModel(PathMIPModel&&) = delete;
    PathMIPModel& operator=(PathMIPModel&&) = delete;

    void setConstraints() override;
    void setObjective() override;
};

#endif