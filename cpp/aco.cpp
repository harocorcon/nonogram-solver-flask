#include "aco.h"
#include "puzzle.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

void PheromoneMatrix::initialize(
    const Puzzle& puzzle,
    double initialValue
) {
    values.resize(puzzle.rows);

    for (int r = 0; r < puzzle.rows; r++) {
        values[r].assign(
            puzzle.rowPatterns[r].size(),
            initialValue
        );
    }
}

double calculateHeuristic(
    const Puzzle& puzzle,
    int row,
    int patternIndex
) {
    double totalCompatibility = 0.0;

    uint32_t rowMask =
        puzzle.rowPatternMasks[row][patternIndex];

    for (int c = 0; c < puzzle.cols; c++) {
        const auto& colDomain =
            puzzle.colDomains[c].patternIndeces;

        if (colDomain.empty()) {
            return 0.0;
        }

        bool rowFilled = (rowMask >> c) & 1u;

        int compatible = 0;

        for (int colPatternIndex : colDomain) {
            uint32_t colMask =
                puzzle.colPatternMasks[c][colPatternIndex];

            bool colFilled = (colMask >> row) & 1u;

            if (rowFilled == colFilled) {
                compatible++;
            }
        }

        totalCompatibility +=
            static_cast<double>(compatible) /
            colDomain.size();
    }

    return totalCompatibility / puzzle.cols;
}

int selectPatternACS(
    const Puzzle& puzzle,
    const PheromoneMatrix& pheromones,
    const ACOConfig& config,
    int row,
    std::mt19937& rng
) {
    const auto& domain =
        puzzle.rowDomains[row].patternIndeces;

    if (domain.empty()) {
        return -1;
    }

    std::vector<double> weights;
    double totalWeight = 0.0;

    int bestPattern = domain[0];
    double bestWeight = -1.0;

    for (int patternIndex : domain) {
        double tau =
            pheromones.values[row][patternIndex];

        double eta =
            calculateHeuristic(puzzle, row, patternIndex);

        double weight =
            std::pow(tau, config.alpha) *
            std::pow(eta, config.beta);

        weights.push_back(weight);
        totalWeight += weight;

        if (weight > bestWeight) {
            bestWeight = weight;
            bestPattern = patternIndex;
        }
    }

    if (totalWeight <= 0.0) {
        std::uniform_int_distribution<int> randomIndex(
            0, static_cast<int>(domain.size()) - 1
        );
        return domain[randomIndex(rng)];
    }

    std::uniform_real_distribution<double> random01(0.0, 1.0);

    // ACS exploitation
    if (random01(rng) <= config.q0) {
        return bestPattern;
    }

    // ACS exploration (roulette-wheel selection)
    std::discrete_distribution<int> distribution(
        weights.begin(),
        weights.end()
    );

    return domain[distribution(rng)];
}


void constructAntSolution(
    const Puzzle& puzzle,
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    Ant& ant,
    std::mt19937& rng
) {
    std::fill(
        ant.selectedPatterns.begin(),
        ant.selectedPatterns.end(),
        -1
    );

    ant.fitness = 0.0;

    for (int r = 0; r < puzzle.rows; r++) {
        int selectedPattern = selectPatternACS(
            puzzle,
            pheromones,
            config,
            r,
            rng
        );

        if (selectedPattern == -1) {
            return;
        }

        ant.selectedPatterns[r] = selectedPattern;

        localPheromoneUpdate(
            pheromones,
            config,
            r,
            selectedPattern
        );
    }
}

std::vector<std::vector<int>> reconstructAntBoard(
    const Puzzle& puzzle,
    const Ant& ant
) {
    std::vector<std::vector<int>> board(
        puzzle.rows,
        std::vector<int>(puzzle.cols, UNKNOWN)
    );

    for (int r = 0; r < puzzle.rows; r++) {
        int patternIndex = ant.selectedPatterns[r];

        if (patternIndex == -1) {
            continue;
        }

        for (int c = 0; c < puzzle.cols; c++) {
            // board[r][c] =
            //     puzzle.rowPatterns[r][patternIndex][c];

            uint32_t mask =
                puzzle.rowPatternMasks[r][patternIndex];
            bool filled = (mask >> c) & 1u;

            board[r][c] = filled ? FILLED : CROSSED;
        }
    }

    return board;
}


double evaluateAntFitness(
    const Puzzle& puzzle,
    const Ant& ant
) {
    auto board = reconstructAntBoard(puzzle, ant);

    int satisfiedColumns = 0;

    for (int c = 0; c < puzzle.cols; c++) {
        uint32_t columnMask = 0;

        for (int r = 0; r < puzzle.rows; r++) {
            if (board[r][c] == FILLED) {
                columnMask |= (1u << r);
            }
        }

        // Check against every valid pattern for this column
        const auto& validMasks =
            puzzle.colPatternMasks[c];

        bool satisfied = std::find(
            validMasks.begin(),
            validMasks.end(),
            columnMask
        ) != validMasks.end();

        if (satisfied) {
            satisfiedColumns++;
        }
    }

    return static_cast<double>(satisfiedColumns)
           / puzzle.cols;
}

bool validateAntSolution(
    const Puzzle &puzzle,
    const Ant& ant
) {
    for (int r = 0; r < puzzle.rows; r++) {
        int selected = ant.selectedPatterns[r];

        if (selected < 0 || selected >= puzzle.rowPatterns[r].size()) {
            return false;
        }
    }
    return evaluateAntFitness(puzzle, ant) == 1.0;
}


void localPheromoneUpdate(
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    int row,
    int patternIndex
) {
    double& tau =
        pheromones.values[row][patternIndex];

    tau = (1.0 - config.localEvaporation) * tau
        + config.localEvaporation
        * config.initialPheromone;
}


void globalPheromoneUpdate(
    PheromoneMatrix& pheromones,
    const ACOConfig& config,
    const Ant& bestAnt
) {
    for (int r = 0; r < static_cast<int>(
        pheromones.values.size()
    ); r++) {
        for (double& tau : pheromones.values[r]) {
            tau *= (1.0 - config.evaporation);
        }

        int selected = bestAnt.selectedPatterns[r];

        if (selected >= 0) {
            pheromones.values[r][selected] +=
                config.evaporation *
                config.Q *
                bestAnt.fitness;
        }
    }
}


std::vector<std::vector<int>> testACS(const Puzzle& puzzle) {
    ACOConfig config;

    PheromoneMatrix pheromones;
    pheromones.initialize(
        puzzle,
        config.initialPheromone
    );

    std::mt19937 rng(42);

    Ant globalBest(puzzle.rows);
    globalBest.fitness = -1.0;

    for (int iteration = 0;
         iteration < config.maxIterations;
         iteration++) {

        for (int i = 0; i < config.numAnts; i++) {
            Ant ant(puzzle.rows);

            constructAntSolution(
                puzzle,
                pheromones,
                config,
                ant,
                rng
            );

            ant.fitness =
                evaluateAntFitness(puzzle, ant);

            if (ant.fitness > globalBest.fitness) {
                globalBest = ant;
            }
        }

        globalPheromoneUpdate(
            pheromones,
            config,
            globalBest
        );

        std::cerr << "Iteration " << iteration + 1
                  << " | Best fitness: "
                  << globalBest.fitness << "\n";

        if (validateAntSolution(puzzle, globalBest)) {
            std::cerr << "SOLVED!\n";
            std::cerr << "\nSelected patterns: ";
            for (int index : globalBest.selectedPatterns) {
                std::cerr << index << " ";
            }

            std::cerr << "\n\nSolved Nonogram:\n";
    
            std::vector<std::vector<int>> board(
                puzzle.rows,
                std::vector<int>(puzzle.cols, UNKNOWN)
            );

            board = reconstructAntBoard(puzzle, globalBest);

            for (const auto& row : board) {
                for (int cell : row) {
                    std::cerr << (cell == FILLED ? "# " : ". ");
                }
                std::cerr << "\n";
            }
            return board;
        }
    }
    std::cerr << "ACS could not find a solution.\n";
    return {};
}


