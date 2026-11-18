#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstdint>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "puzzle.h"
#include "aco.h"

uint32_t patternToMask(const std::vector<int>& pattern) {
    uint32_t mask = 0;
    for(int i = 0; i < pattern.size() && i < 32; i++) {
        if(pattern[i] == FILLED) {
            mask |= (1 << i);
        }
    }
    return mask;
}

uint32_t computeAndMask(const std::vector<uint32_t>& patterns) {
    if(patterns.empty()) return 0;

    uint32_t result = patterns[0];
    for(int i = 1; i < patterns.size(); i++) {
        result &= patterns[i];
    }
    return result;
}

uint32_t computeOrMask(const std::vector<uint32_t>& patterns) {
    if (patterns.empty()) return 0xFFFFFFFF;

    uint32_t result = patterns[0];
    for(int i = 1; i < patterns.size(); i++) {
        result |= patterns[i];
    }
    return result;
}

void clearBoard(Puzzle &puzzle) {
    std::cout << "Clearing board of size " << puzzle.rows << "x" << puzzle.cols << "...\n";
    for(int r = 0; r < puzzle.rows; r++) {
        for(int c = 0; c < puzzle.cols; c++){
            puzzle.board[r][c] = UNKNOWN;
        }
    }
}

int minimumSpaceRequired(const std::vector<int>& clues, int clueIndex) {
    int space = 0;

    for(int i = clueIndex; i < clues.size(); i++){
        space += clues[i];
    }

    // one gap between every remaining pair of clues
    int remainingClues = clues.size() - clueIndex;
    space += remainingClues - 1;

    return space;

}

void generatePatternsRecursive(
    int length,
    const std::vector<int>& clues,
    int clueIndex,
    int position,
    std::vector<int>& currentPattern,
    std::vector<std::vector<int>>& patterns
) {
    // Base case: if all clues have been placed
    if(clueIndex == clues.size()) {
        patterns.push_back(currentPattern);
        return;
    }

    int blockSize = clues[clueIndex];

    // Try placing this block at every possible position
    int minSpace = minimumSpaceRequired(clues, clueIndex);
    for(int start = position; start <= length - minSpace; start++) {
        // make sure the block fits with enough room for the remaining clues
        for( int i = 0; i < blockSize; i++) {
            currentPattern[start + i] = FILLED;
        }
        // move past this block and leave one empty cell
        int nextPosition = start + blockSize + 1;

        generatePatternsRecursive(length, clues, clueIndex + 1, nextPosition, currentPattern, patterns);

        // undo the placement
        for(int i = 0; i < blockSize; i++) {
            currentPattern[start + i] = UNKNOWN;
        }
    }
}

std::vector<std::vector<int>> generatePatterns(
    int length,
    const std::vector<int>& clues
)
{
    std::vector<std::vector<int>> patterns;
    
    std::vector<int> currentPattern(length, UNKNOWN);

    generatePatternsRecursive(length, clues, 0, 0, currentPattern, patterns);

    return patterns;
}

void printBoard(const Puzzle& puzzle)
{
    for(const auto& row : puzzle.board)
    {
        for(int cell : row)
        {
            std::cout << cell << " ";
        }

        std::cout << "\n";
    }
}

Puzzle readPuzzle(int rows, int cols) {
    Puzzle puzzle;
    puzzle.rows = rows;
    puzzle.cols = cols;
    puzzle.board.resize(rows, std::vector<int>(cols, UNKNOWN));

    std::string line;
    std::getline(std::cin, line);
    
    for (int r = 0; r < rows; r++){
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::vector<int> clue;

        int value;

        while (ss >> value)
        {
            clue.push_back(value);
        }

        puzzle.rowClues.push_back(clue);
    }

    for (int c = 0; c < cols; c++) {
        std::string line;
        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::vector<int> clue;

        int value;

        while (ss >> value)
        {
            clue.push_back(value);
        }

        puzzle.colClues.push_back(clue);
    }

    puzzle.board = std::vector<std::vector<int>>(
        puzzle.rows,
        std::vector<int>(puzzle.cols, UNKNOWN)
    );   
    
    return puzzle;
}


//  Copilot
// The initializeDomains function initializes the row and column domains of a Puzzle
// object by resizing the rowDomains and colDomains vectors, setting up possible pattern
// indices, and computing the AND and OR masks for each row and column based on 
// their respective pattern masks. This setup is essential for managing the 
// constraints and possibilities within the puzzle's structure.
void initializeDomains(Puzzle& puzzle) { 
    puzzle.rowDomains.resize(puzzle.rows);
    puzzle.colDomains.resize(puzzle.cols);

    // intialize row domains
    for (int r = 0; r < puzzle.rows; r++) {
        Domain& d = puzzle.rowDomains[r];

        // All pattern indeces are initally possible
        d.patternIndeces.resize(puzzle.rowPatterns[r].size());
        for( int i = 0; i < d.patternIndeces.size(); i++) {
            d.patternIndeces[i] = i;
        }

        d.andMask = computeAndMask(puzzle.rowPatternMasks[r]);
        d.orMask = computeOrMask(puzzle.rowPatternMasks[r]);
    }

    // initialize column domains
    for( int c= 0; c < puzzle.cols; c++) {
        Domain& d = puzzle.colDomains[c];

        d.patternIndeces.resize(puzzle.colPatterns[c].size());
        for(int i = 0; i < d.patternIndeces.size(); i++) {
            d.patternIndeces[i] = i;
        }

        d.andMask = computeAndMask(puzzle.colPatternMasks[c]);
        d.orMask = computeOrMask(puzzle.colPatternMasks[c]);
    }
}

// From paper SECTION 3.4
// "These two tuples are used to filter the domains of intersecting
// row/column variables"
void filterRowPatterns(Puzzle& puzzle, int row) {
    std::vector<int> newDomain;

    for (int patternIndex : puzzle.rowDomains[row].patternIndeces) {
        uint32_t rowMask = puzzle.rowPatternMasks[row][patternIndex];
        bool valid = true;
        int rejectCol = -1;
        const char* reason = "";

        for(int c = 0; c < puzzle.cols; c++) {
            // check 1: column guarantees FILLED at this row
            if (puzzle.colDomains[c].andMask & (1u << row)) {
                if (!(rowMask & (1u << c))) {
                    valid = false;
                    rejectCol = c;
                    reason = "\nColumn guarantees FILLED but pattern is EMPTY";
                    break;
                }
            }
            // check 2: column guarantees EMPTY at this row
            // OR-tuple convention: bit = 0 guarantees EMPTY
            if (!(puzzle.colDomains[c].orMask & (1u << row))) {
                if (rowMask & (1u << c)) {
                    valid = false;
                    rejectCol = c;
                    reason = "\nColumn guarantees EMPTY but pattern is FILLED";
                    break;
                }
            }
        }

        std::cerr << "\npattern " << patternIndex << "[";
        for(int c = 0; c < puzzle.cols; c++)
            std::cerr << ((rowMask >> c) & 1);
        if(!valid)
            std::cerr << "(col " << rejectCol << ": " << reason << ")";
        
        if (valid) newDomain.push_back(patternIndex);
    }
    puzzle.rowDomains[row].patternIndeces = newDomain;
}

void filterColPatterns(Puzzle& puzzle, int col) {
    std::vector <int> newDomain;

    for (int patternIndex : puzzle.colDomains[col].patternIndeces) {
        uint32_t colMask = puzzle.colPatternMasks[col][patternIndex];
        bool valid = true;

        for (int r = 0; r < puzzle.rows; r++) {
            // check 1: row guarantees FILLED at this col
            if (puzzle.rowDomains[r].andMask & (1u << col)) {
                if(!(colMask & (1u << r))){
                    valid = false;
                    break;
                }
            } 
            // check 2: row guarantees EMPTY at this col
            if(!(puzzle.rowDomains[r].orMask & (1u << col))) {
                if (colMask & (1u << r)) {
                    valid = false;
                    break;
                }
            }
        }
        if(valid) newDomain.push_back(patternIndex);
    }
    puzzle.colDomains[col].patternIndeces = newDomain;
}

// recompute AND/OR masks for a domain from its curent pattern indeces
void recomputeRowMasks(Puzzle& puzzle, int row) {
    std::vector<uint32_t> currentMasks;
    currentMasks.reserve(puzzle.rowDomains[row].patternIndeces.size());

    for(int index: puzzle.rowDomains[row].patternIndeces) {
        currentMasks.push_back(puzzle.rowPatternMasks[row][index]);
    }

    puzzle.rowDomains[row].andMask = computeAndMask(currentMasks);
    puzzle.rowDomains[row].orMask = computeOrMask(currentMasks);
}

void recomputeColumnMasks(Puzzle& puzzle, int col) {
    std::vector<uint32_t> currentMasks;
    currentMasks.reserve(puzzle.colDomains[col].patternIndeces.size());

    for(int index: puzzle.colDomains[col].patternIndeces) {
        currentMasks.push_back(puzzle.colPatternMasks[col][index]);
    }

    puzzle.colDomains[col].andMask = computeAndMask(currentMasks);
    puzzle.colDomains[col].orMask = computeOrMask(currentMasks);
}

// Section 3.4
// This iterative process significantly narrows down the domains of the variables and continues until no further reduction can be made.

// returns true if the puzzle became fully solved (all domains =  1 pattern)
bool propagateAllConstraints(Puzzle& puzzle, bool verbose = false) {
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 200;

    while( changed && iteration < MAX_ITERATIONS) {
        changed = false;
        iteration++;

        if(verbose){
            std::cerr << "\n----Propagation iteration " << iteration << "----" << std::endl;
        }

        // 1. Filter rows using current column masks
        for(int r = 0; r < puzzle.rows; r++) {
            int oldSize = puzzle.rowDomains[r].patternIndeces.size();
            filterRowPatterns(puzzle, r);
            int newSize = puzzle.rowDomains[r].patternIndeces.size();
            
            if(newSize < oldSize) {
                changed = true;
                recomputeRowMasks(puzzle, r);
                if(verbose) {
                    std::cerr << "Row " << r << ":" << oldSize << " -> " << newSize << " patterns" << std::endl;
                }
            }
        }

        // 2. Filter columns using updated row masks
        for(int c = 0; c < puzzle.cols; c++) {
            int oldSize = puzzle.colDomains[c].patternIndeces.size();
            filterColPatterns(puzzle, c);
            int newSize = puzzle.colDomains[c].patternIndeces.size();

            if(newSize != oldSize) {
                changed = true;
                recomputeColumnMasks(puzzle, c);
                if(verbose) {
                    std::cerr << "Col " << c << ": " << oldSize << " -> " << newSize << " patterns" << std::endl;
                }
            }
        }
    }

    // check if solved
    for(int r = 0; r < puzzle.rows; r++) {
        if(puzzle.rowDomains[r].patternIndeces.size() != 1) return false;
    }

    for(int c = 0; c < puzzle.cols; c++) {
        if(puzzle.colDomains[c].patternIndeces.size() != 1) return false;
    }

    return true;
}

// call only after propagateAllConstraints returns true
void reconstructBoardFromDomains(Puzzle& puzzle) {
    for(int r = 0; r < puzzle.rows; r++) {
        int patternIndex = puzzle.rowDomains[r].patternIndeces[0];

        for(int c = 0; c < puzzle.cols; c++) {
            int cell = puzzle.rowPatterns[r][patternIndex][c] == UNKNOWN ? CROSSED : puzzle.rowPatterns[r][patternIndex][c];
            puzzle.board[r][c] = cell;
        }
    }
}

void printAmbiguousDomains(const Puzzle& puzzle) {
    std::cerr << "\nAmbiguous rows/columns (domain > 1): " << std::endl;
    bool anyAmbiguous = false;

    for (int r = 0; r < puzzle.rows; r++) {
        size_t n = puzzle.rowDomains[r].patternIndeces.size();
        if(n > 1) {
            std::cerr << " Row " << r << ": " << n << " patterns remain" << std:: endl;
            anyAmbiguous = true;
        } else if( n == 0 ) {
            std::cerr << " Row " << r << ": EMPTY DOMAIN (contradiction!)" << std::endl;
            anyAmbiguous = true;
        }
    }
    
    for(int c = 0; c < puzzle.cols; c++) {
        size_t n = puzzle.colDomains[c].patternIndeces.size();
        
        if(n > 1) {
            std::cerr << " Col " << c << ": " << n << " patterns remain" << std::endl;
            anyAmbiguous = true;
        } else if( n == 0 ) {
            std::cerr << " Col " << c << ": empty domain!" << std::endl;
            anyAmbiguous = true;
        }
    }

    if(!anyAmbiguous) {
        std::cerr << " (none - puzze is solved)" << std::endl;
    }
}

 enum SolveMethod {
    PROPAGATION_ONLY,
    PROPAGATION_WITH_BACKTRACK,
    PROPAGATION_WITH_ACO,
    PROPAGATION_WITH_ACO_SA,
    ACO_ONLY
 };

 // From paper Section 3.4: 
 // " Our approach relies on a fixed ordering of variables: R1, C1, R2, C2, ..."
 // "variable" here is a row or a column. we represent each as a small struct
 enum VarKind { VAR_ROW, VAR_COL };

 struct Var {
    VarKind kind;
    int index; // row index or column index
 };

 // build the interleaved ordering R0, C0, R1, C1, ....
 // If rows and columns differ in count, the leftover ones are appended at the end.
 std::vector<Var> buildVarOrdering(int rows, int cols) {
    std::vector<Var> order;
    int maxLen = std::max(rows, cols);
    for(int i = 0; i < maxLen; i++) {
        if(i < rows) order.push_back({VAR_ROW, i});
        if(i < cols) order.push_back({VAR_COL, i});
    }
    return order;
 }

 struct Assignment {
    // for each row: pattern index, or -1 if unassigned
    std::vector<int> rowPattern;
    // for each col: pattern index, or -1 if unassigned
    std::vector<int> colPattern;
    int assignedCount = 0;

    Assignment(int rows, int cols) {
        rowPattern.assign(rows, -1);
        colPattern.assign(cols, -1);
    }
 };

 // check whether a candidate pattern for variable V is 
 // consistent with all currently assigned variables that intersect V
bool isConsistent(
    const Puzzle& puzzle,
    const Assignment& assign,
    Var V,
    int candidatePatternIndex
 ) {
    if(V.kind == VAR_ROW) {
        // candidate is a row pattern. check against every assigned column.
        uint32_t rowMask = puzzle.rowPatternMasks[V.index][candidatePatternIndex];

        for(int c = 0; c < puzzle.cols; c++) {
            int colPatternIndex = assign.colPattern[c];
            if(colPatternIndex == -1) continue; // column not assigned yet

            uint32_t colMask = puzzle.colPatternMasks[c][colPatternIndex];

            // shared cell is (V.index, c)
            // row says: (rowMask >> c) & 1
            // col says: (colMask >> V.index) & 1
            bool rowFilled = (rowMask >> c) & 1u;
            bool colFilled = (colMask >> V.index) & 1u;

            if(rowFilled != colFilled) return false;
        }
        return true;
    } else {
        // candidate is a column pattern. check against every assigned row.
        uint32_t colMask = puzzle.colPatternMasks[V.index][candidatePatternIndex];

        for(int r = 0; r < puzzle.rows; r++) {
            int rowPatternIndex = assign.rowPattern[r];
            if(rowPatternIndex == -1) continue; // row not assigned yet

            uint32_t rowMask = puzzle.rowPatternMasks[r][rowPatternIndex];

            // shared cell is (r, V.index)
            bool rowFilled = (rowMask >> V.index) & 1u;
            bool colFilled = (colMask >> r) & 1u;
            if(rowFilled != colFilled) return false;
        }
        return true; 
    }
 }

 // Global-ish counters for benchmarking
 struct BacktrackStats {
    long long nodesVisited = 0;         // recursion calls
    long long consistencyChecks = 0;    // how many isConsistent() calls
    long long backtracks = 0;           // how many times we undid
 };

bool backtrackRecursive(
    const Puzzle& puzzle,
    const std::vector<Var>& order,
    Assignment& assign,
    int depth,
    BacktrackStats& stats
) {
    stats.nodesVisited++;

    // base case: all variables are assigned
    if(assign.assignedCount == (int)order.size()) {
        return true; // solved!
    }

    // choose the next variable
    Var V = order[depth];

    const std::vector<int>& domain = (V.kind == VAR_ROW)
        ? puzzle.rowDomains[V.index].patternIndeces
        : puzzle.colDomains[V.index].patternIndeces;

    // try every pattern from the domain
    for(int candidateIndex : domain) {
        stats.consistencyChecks++;
        if(!isConsistent(puzzle, assign, V, candidateIndex)) {
            continue;
        }

        if(V.kind == VAR_ROW) 
            assign.rowPattern[V.index] = candidateIndex;
        else
            assign.colPattern[V.index] = candidateIndex;
        assign.assignedCount++;

        if(backtrackRecursive(puzzle, order, assign, depth + 1, stats))
            return true;

        if(V.kind == VAR_ROW)
            assign.rowPattern[V.index] = -1;
        else    
            assign.colPattern[V.index] = -1;
        
        assign.assignedCount--;
        stats.backtracks++;
    }
    return false;
}

bool backtrackingSolve(Puzzle& puzzle, BacktrackStats& stats) {
    std::vector<Var> order = buildVarOrdering(puzzle.rows, puzzle.cols);
    Assignment assign(puzzle.rows, puzzle.cols);

    stats = BacktrackStats{};

    bool solved = backtrackRecursive(puzzle, order, assign, 0, stats);

    if(solved) {
        for(int r = 0; r < puzzle.rows; r++) {
            int pIndex = assign.rowPattern[r];
            for(int c = 0; c < puzzle.cols; c++) {
                puzzle.board[r][c] = puzzle.rowPatterns[r][pIndex][c] == UNKNOWN? CROSSED : puzzle.rowPatterns[r][pIndex][c];
            }
        }
    }

    return solved;
}

void generateAllPatterns(Puzzle& puzzle) {
    // --- ROWS ---
    puzzle.rowPatterns.resize(puzzle.rows);
    puzzle.rowPatternMasks.resize(puzzle.rows);   
    
    for (int r = 0; r < puzzle.rows; r++) {
        puzzle.rowPatterns[r] = generatePatterns(puzzle.cols, puzzle.rowClues[r]);
        puzzle.rowPatternMasks[r].clear();
        for (const auto& p : puzzle.rowPatterns[r])
            puzzle.rowPatternMasks[r].push_back(patternToMask(p));
    }
    
    // --- COLUMNS ---
    puzzle.colPatterns.resize(puzzle.cols);       // use puzzle.cols
    puzzle.colPatternMasks.resize(puzzle.cols);
    
    for (int c = 0; c < puzzle.cols; c++) {
        puzzle.colPatterns[c] = generatePatterns(puzzle.rows, puzzle.colClues[c]);
        puzzle.colPatternMasks[c].clear();
        for (const auto& p : puzzle.colPatterns[c])
            puzzle.colPatternMasks[c].push_back(patternToMask(p));
    }
}

bool solvePuzzle(Puzzle& puzzle, SolveMethod method, bool verbose) {
    std::cerr << "Generating all patterns..." << std::endl;
    
    generateAllPatterns(puzzle);
    initializeDomains(puzzle);
    
    switch (method) {
        case PROPAGATION_ONLY: {
            std::cerr << "\n[METHOD] PROPAGATION_ONLY -- stopping here." << std::endl;
            bool propagation = propagateAllConstraints(puzzle, verbose);
            
            if(propagation) {
                std::cerr << "\n[SUCESS] Solved by propagation alone," << std::endl;
                reconstructBoardFromDomains(puzzle);
                return true;
            }
            else{
                std::cerr << "\n[STUCK] Propagation could not fully solve." << std::endl;
              // printAmbiguousDomains(puzzle);
            }
            return propagation;
        }

        case PROPAGATION_WITH_BACKTRACK: {
            std::cerr << "\n[METHOD] PROPAGATION_WITH_BACKTRACK" << std::endl;
            bool propagation = propagateAllConstraints(puzzle, verbose);
            if(propagation) {
                std::cerr << "\n[SUCESS] Solved by propagation alone," << std::endl;
                reconstructBoardFromDomains(puzzle);
                return true;
            }
            
            BacktrackStats stats;
            bool solved = backtrackingSolve(puzzle, stats);

            std::cerr << "Solved: " << (solved ? "YES" : "NO") << std::endl;
            std::cerr << "Nodes visited: " << stats.nodesVisited << std::endl;
            std::cerr << "Consistency checks: " << stats.consistencyChecks << std::endl;
            std::cerr << "Backtracks: " << stats.backtracks << std::endl;

            return solved;
        }

        case PROPAGATION_WITH_ACO: {
            std::cerr << "\n[METHOD] PROPAGATION_WITH_ACO" << std::endl;
            bool propagation = propagateAllConstraints(puzzle, verbose);
            
            if(propagation) {
                std::cerr << "\n[SUCESS] Solved by propagation alone," << std::endl;
                reconstructBoardFromDomains(puzzle);
                return true;
            }
            else{
                std::cerr << "\n[STUCK] Propagation could not fully solve." << std::endl;
            }

            auto solvedBoard = testACS(puzzle);

            if(!solvedBoard.empty()) {
                puzzle.board = solvedBoard;
                std::cerr << "ACS solution stored in puzzle.board \n";
            } else {
                std::cerr << "ACS failed to find a solution.\n";
            }
            return !solvedBoard.empty();
        }

        case ACO_ONLY: {
            std::cerr << "\n[METHOD] ACO_ONLY." << std::endl;
            auto solvedBoard = testACS(puzzle);

            if(!solvedBoard.empty()) {
                puzzle.board = solvedBoard;
                std::cerr << "ACS solution stored in puzzle.board \n";
            } else {
                std::cerr << "ACS failed to find a solution.\n";
            }
            return !solvedBoard.empty();
        }

        case PROPAGATION_WITH_ACO_SA:
            std::cerr << "\n[METHOD] PROPAGATION_WITH_ACO_SA not yet implemented." << std::endl;
            return false;
        default:
            std::cerr << "Unknown method." << std::endl;
            return false;
    }

    return true;
}

int main(int argc, char* argv[]) {

    if(argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <method> <rows> <cols>" << argv[1] << std:: endl;
        std::cerr << " method: propagation | backtrack | aco | aco_sa" << std::endl;
        return 1;
    }
    std::string action = argv[1];
    std::string methodStr = argv[2];
    int rows = std::stoi(argv[3]);
    int cols = std::stoi(argv[4]);

    auto start = std::chrono::high_resolution_clock::now();
    
    SolveMethod method;
    if(methodStr == "cp")
        method = PROPAGATION_ONLY;
    else if ( methodStr == "cp-bt")
        method = PROPAGATION_WITH_BACKTRACK;
    else if (methodStr == "cp-aco") 
        method = PROPAGATION_WITH_ACO;
    else if (methodStr == "cp-aco-sa")
        method = PROPAGATION_WITH_ACO_SA;
    else if (methodStr == "aco") 
        method = ACO_ONLY;
    else{
        std::cerr << "Unknown method: " << methodStr << std::endl;
        return 1; 
    }

    Puzzle puzzle = readPuzzle(rows, cols);
    
    bool solved = solvePuzzle(puzzle, method, true);
    if(solved) printBoard(puzzle);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cerr << "Total time: " << ms << " ms" << std::endl;
    
    return 0;
}

