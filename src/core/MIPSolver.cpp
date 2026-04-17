#include "MIPSolver.h"

#include <algorithm>

MIPSolver::MIPSolver() {}

MIPSolver::~MIPSolver() {
    for(MIPForest* F: forests) delete F;
}

void MIPSolver::solve(Instance& instance) {
    forests = instance.mipTrees();

    instanceName = instance.title();

    size_t index = 1;

    MIPForest* F = forests.front();
    
    while(index < forests.size())
        solveFor(F, forests[index++]);
    
    instance.exportOutput(F);
}

void MIPSolver::solveFor(MIPForest* MAFCandidate, MIPForest* F) {
    // Inicializa modelo
    // mip = std::make_unique<PathMIPModel>(MAFCandidate, F);
    mip = std::make_unique<PairMIPModel>(MAFCandidate, F);

    // MAFCandidate->printAdjAndParents();
    // MAFCandidate->printEdgeIds();
    // std::cout << "----\n";
    // F->printAdjAndParents();
    // F->printEdgeIds(); 

    GreedySolver greedy = GreedySolver(MAFCandidate, F);
    greedy.solveUsingConstraintsScore();
    std::unordered_set<int> edgesF1 = greedy.edgesToCutF1();

    // MIP generation
    mip->generateVariables();
    mip->addPrimalHeuristic(edgesF1);
    mip->setConstraints();
    mip->setObjective();

    // Solve
    mip->solve();
    // mip->exportSolution();

    // Write stats
    // const std::string csvPath = "./edgeScore/" + instanceName + ".csv";
    // std::ifstream existingCsv(csvPath);
    // const bool writeHeader = !existingCsv.good() || existingCsv.peek() == std::ifstream::traits_type::eof();
    // existingCsv.close();

    // std::ofstream csv(csvPath, std::ios::app);

    // if (!csv.is_open()) {
    //     std::cerr << "Could not open CSV file: " << csvPath << "\n";
    // } else {
    //     if (writeHeader) 
    //         csv << "edge_id,number,edge_score,triples,paths,damage,d_value,leaf\n";

    //     csv << std::fixed << std::setprecision(6);

    //     for(const EdgeInfo& e: greedy.edgesInfo) {

    //         int id = MAFCandidate->edgeForNode(e.child, e.parent);

    //         csv << id 
    //             << "," << e.edgeScore 
    //             << "," << e.number
    //             << "," << e.tripleScore
    //             << "," << e.pathScore
    //             << "," << e.damage
    //             << "," << (id != -1 ? mip->getValueFor(MAFCandidate->modId(), id) : -1)
    //             << "," << e.isLeaf << "\n";
    //     }
    // }
    

    pruneAndRegraft(MAFCandidate);
}

void MIPSolver::pruneAndRegraft(MIPForest* F) const {

    // Cutting and Regraft
    for(int e = 0; e < F->amountOfEdges(); e++) {
        int cutting = mip->getValueFor(F->modId(), e);

        if (not cutting or not F->edgeIsAvailable(e)) continue;

        auto [descendant, ancestor] = F->nodesOf(e);

        F->cut(descendant);
    }

    F->regraft();

}
