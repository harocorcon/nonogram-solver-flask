#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstdint>

const int UNKNOWN = 0;
const int FILLED = 1;
const int CROSSED = -1;

struct Domain {
    std::vector<int> patternIndeces; // possible patterns
    uint32_t andMask; // AND of all patterns
    uint32_t orMask;  // OR of all patterns
};
struct Puzzle {
    int rows;
    int cols;

    std::vector<std::vector<int>> rowClues;
    std::vector<std::vector<int>> colClues;
    std::vector<std::vector<int>> board;

    std::vector<std::vector<std::vector<int>>> rowPatterns;  // [row][patternIndex][cell]   
    std::vector<std::vector<std::vector<int>>> colPatterns;  // [col][patternIndex][cell]   

    std::vector<int> rowPatternCounts;
    std::vector<int> colPatternCounts;

    std::vector<std::vector<uint32_t>> rowPatternMasks;
    std::vector<std::vector<uint32_t>> colPatternMasks;
    std::vector<Domain> rowDomains; 
    std::vector<Domain> colDomains;
};

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



void solvePuzzle(Puzzle& puzzle) {
    for (int r = 0; r < puzzle.rows; r++)
    {
        for (int c = 0; c < puzzle.cols; c++)
        {
            puzzle.board[r][c] = FILLED;
        }
    }

    std::vector<int> clues = {2, 1};

    std::vector<std::vector<int>> patterns = generatePatterns(5, clues);

    for(const auto& pattern : patterns) {
        for (int cell : pattern) {
            std::cerr << cell << "-";
        }
        std::cerr << "\n";
    }

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
    std::cerr << "Filtering row patterns for row " << row << "domain size = " << puzzle.rowDomains[row].patternIndeces.size() << " -- " << std::endl;
    
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
                    reason = "Column guarantees FILLED but pattern is EMPTY";
                    break;
                }
            }
            // check 2: column guarantees EMPTY at this row
            // OR-tuple convention: bit = 0 guarantees EMPTY
            if (!(puzzle.colDomains[c].orMask & (1u << row))) {
                if (rowMask & (1u << c)) {
                    valid = false;
                    rejectCol = c;
                    reason = "Column guarantees EMPTY but pattern is FILLED";
                    break;
                }
            }
        }

        std::cerr << "\npattern " << patternIndex << "[";
        for(int c = 0; c < puzzle.cols; c++)
            std::cerr << ((rowMask >> c) & 1);
        std::cerr << "] -> " << (valid ? "KEEP" : "REJECT");
        if(!valid)
            std::cerr << "(col " << rejectCol << ": " << reason << ")";
        
        if (valid) newDomain.push_back(patternIndex);
    }
    puzzle.rowDomains[row].patternIndeces = newDomain;
    std::cerr << " -> new domain size: " << newDomain.size() << std::endl;
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

// returns true of the puzzle became fully solved (all domains =  1 pattern)
bool propagateAllConstraints(Puzzle& puzzle, bool verbose = false) {
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 200;

    while( changed && iteration < MAX_ITERATIONS) {
        changed = false;
        iteration++;

        if(verbose){
            std::cerr << "/n----Propagation iteration " << iteration << "----" << std::endl;
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
            puzzle.board[r][c] = puzzle.rowPatterns[r][patternIndex][c];
        }
    }
}

int main(int argc, char* argv[]) {
    std::string action = argv[1];
    int rows = std::stoi(argv[2]);
    int cols = std::stoi(argv[3]);

    Puzzle puzzle = readPuzzle(rows, cols);
    // Puzzle puzzle;
    if(action == "solve") {
        // solvePuzzle(puzzle);
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return 1;
    }

    printBoard(puzzle);
    
    return 0;
}

