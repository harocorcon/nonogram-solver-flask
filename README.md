# Nonogram Solver — Research Prototype

An undergraduate thesis research prototype exploring Nonogram solving through **Constraint Propagation (CP)**, **backtracking**, and an initial **Ant Colony System (ACS)** implementation. The planned research direction includes **Simulated Annealing (SA)** and parallel cooperation between the methods.

> **Status:** Work in progress. The implemented solver can solve selected puzzles, but it does not guarantee a solution for every puzzle within the configured search limits.

## Overview

Nonograms are logic puzzles defined by clues for each row and column. This project generates clue-compatible patterns, applies constraint-based reductions, and explores search methods for constructing a grid that satisfies all clues.

The application uses a **C++ solving engine**, a **Flask (Python) backend**, and an **HTML/JavaScript frontend**.

## Implemented functionality

- Generation of valid row and column patterns from clues
- Constraint propagation to eliminate incompatible patterns
- Backtracking solver as a baseline
- Initial ACS solver with pheromone-weighted and heuristic-guided pattern selection
- Ant candidate construction and fitness evaluation
- Local and global pheromone updates across iterations
- Validation of completed grids and display of solutions in the frontend

## Planned work

- Improve the ACS fitness function and heuristic
- Integrate Simulated Annealing
- Investigate parallel execution and information exchange
- Benchmark solving success, runtime, and search effort across methods

## How the ACS prototype works

1. Generate patterns satisfying each row and column clue.
2. Initialize domains; optionally apply constraint propagation.
3. Each ant selects one candidate pattern per row using ACS-style selection.
4. Evaluate whether the resulting grid satisfies column constraints.
5. Apply pheromone updates and repeat until a valid solution is found or the iteration limit is reached.
6. Return a solved board to the Flask application when successful.

**Note:** The initial fitness score is the fraction of columns that satisfy their clues. A score of `1.0` indicates a valid solution only when all row selections are valid and the full board passes validation.

## Demonstration

- **Short video:** 

## Running locally

> Replace the placeholders below with the actual filenames, paths, and commands from this repository before publishing.

### Requirements

- A C++17-compatible compiler (such as `g++`)
- Python 3 and Flask
- Other Python packages listed in `requirements.txt` (if present)

### Build the C++ solver

```bash
# Example only — adjust source filenames and paths
 g++ -std=c++17 test.cpp aco.cpp -o nonogram
```

If the solver includes additional `.cpp` modules, add them to the build command. On Windows, the output executable may be named `nonogram.exe`.

### Run the Flask application

```bash
# Example only — adjust for the actual Flask entry point
py app.py
```

Open the local address printed by Flask. The Flask application must point to the compiled solver executable appropriate for your operating system.

## Testing and limitations

- CP may fully solve highly constrained puzzles before ACS begins.
- ACS without CP explores more candidate patterns and may fail within the configured iteration limit.
- Current results are preliminary; a systematic benchmark has not yet been completed.
- The current ACS implementation is serial; SA and parallel cooperation are not yet implemented.
- Solver success and runtime can depend on puzzle difficulty, random seed, ant count, and iteration limits.

## Research status

This repository documents implementation progress for adviser review. It is **not yet a completed or experimentally validated hybrid CP–ACS–SA solver**.

## Author

Hannah Corcilles
201139372

