#ifndef LAZYMIPMODEL_H
#define LAZYMIPMODEL_H

#include "MIPModel.h"

struct Path {
    int tree,i,j,k,l;
    Path(int tree, int i, int j, int k, int l): tree(tree), i(i), j(j), k(k), l(l) {}
};

class LazyMIPModel: public MIPModel {
protected:

    size_t constriantBound;
    
public:
    LazyMIPModel(): MIPModel(), constriantBound(10) {};
    LazyMIPModel(MIPForest* F1, MIPForest* F2): MIPModel(F1, F2), constriantBound(std::max(int(F1->amountOfTriples() * 0.1), 10)) {};
    virtual ~LazyMIPModel() {};
    LazyMIPModel(const LazyMIPModel&) = delete;
    LazyMIPModel& operator=(const LazyMIPModel&) = delete;
    LazyMIPModel(MIPModel&&) = delete;
    MIPModel& operator=(MIPModel&&) = delete;

    virtual void setBasicConstraints() = 0;

    virtual bool searchForIncompatiblePaths(MIPForest* A, MIPForest* B) = 0;

};




#endif