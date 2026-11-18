
#ifndef ACO_H
#define ACO_H

#include <vector>
#include <random>
#include <cstdint>

struct Puzzle;

struct ACOConfig {
    int numAnts = 20;
    int maxIterations = 100;

    double alpha = 1.0;
    double beta = 2.0;
    double evaporation = 0.1;
    double localEvaporation = 0.1;
    double initialPheromone = 1.0;
    double q0 = 0.9;
    double Q = 1.0;
};

struct Ant {
    std::vector<int> selectedPatterns;
    double fitness = 0.0;

    explicit Ant(int rows)
        : selectedPatterns(rows, -1) {}
};

struct PheromoneMatrix {
    std::vector<std::vector<double>> values;

    void initialize(
        const Puzzle& puzzle,
        double initialValue
    );
};

// Ant construction
double calculateHeuristic(
    const Puzzle& puzzle,
    int row,
    int patternIndex
);

int selectPatternACS(
    const Puzzle& puzzle,
    const PheromoneMatrix& pheromones,
    const ACOConfig& config,
    int row,
    std::mt19937& rng
);

void constructAntSolution(
    const Puzzle& puzzle,
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    Ant& ant,
    std::mt19937& rng
);

// Fitness evaluation
std::vector<std::vector<int>> reconstructAntBoard(
    const Puzzle& puzzle,
    const Ant& ant
);

double evaluateAntFitness(
    const Puzzle& puzzle,
    const Ant& ant
);

bool validateAntSolution(
    const Puzzle& puzzle,
    const Ant& ant
);

// Pheromone updates
void localPheromoneUpdate(
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    int row,
    int patternIndex
);

void globalPheromoneUpdate(
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    const Ant& bestAnt
);

std::vector<std::vector<int>> testACS(const Puzzle& puzzle);

#endif
