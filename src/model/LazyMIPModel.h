#ifndef LAZYMIPMODEL_H
#define LAZYMIPMODEL_H

#include "MIPModel.h"

class LazyCallbackI;

class LazyMIPModel: public MIPModel {
protected:

    size_t triplesBound, pathsBound;
    
public:
    LazyMIPModel(): MIPModel(), triplesBound(10), pathsBound(10) {};
    LazyMIPModel(MIPForest* F1, MIPForest* F2): MIPModel(F1, F2), triplesBound(std::max(int(F1->amountOfTriples() * 0.2), 10)), pathsBound(std::max(int(F1->amountOfTriples() * 0.2), 10)) {};
    virtual ~LazyMIPModel() {};
    LazyMIPModel(const LazyMIPModel&) = delete;
    LazyMIPModel& operator=(const LazyMIPModel&) = delete;
    LazyMIPModel(MIPModel&&) = delete;
    MIPModel& operator=(MIPModel&&) = delete;

    virtual bool searchForConflictiveTriples(const LazyCallbackI& callback, std::vector<Triple>& constraintToAdd) = 0;
    virtual bool searchForIncompatiblePaths(const LazyCallbackI& callback, std::vector<Path>& constraintToAdd) = 0;

    virtual int getCallbackValueFor(const LazyCallbackI& callback, int forestId, int i, int j) const = 0;

};




#endif
