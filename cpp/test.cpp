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

void testGenerateAllPatterns() {
    Puzzle puzzle;
    puzzle.rows = 8;
    puzzle.cols = 8;
    puzzle.board = std::vector<std::vector<int>>(puzzle.rows, std::vector<int>(puzzle.cols, UNKNOWN));

    puzzle.rowClues = {
        {3},
        {1, 4, 1},
        {3, 1},
        {1, 2},
        {5, 2},
        {3, 2, 1},
        {8},
        {3, 3}
    };
    puzzle.colClues = {
        {3, 1},
        {1, 2},
        {2, 1},
        {3},
        {1, 4, 1},
        {3, 1},
        {1, 2},
        {5, 2},
        {2, 1}
    };

    std::cerr << "Testing pattern generation " << std::endl;
    puzzle.rowPatterns.resize(puzzle.rows);
    for(int r = 0; r < puzzle.rows; r++) {
        puzzle.rowPatterns[r] = generatePatterns(puzzle.cols, puzzle.rowClues[r]);
        std::cerr << "Row " << r << " patterns: " << puzzle.rowPatterns[r].size() << std::endl;
        for(int c : puzzle.rowClues[r]) {
            std::cerr << c << " ";
        }
        std::cerr << "-> " << puzzle.rowPatterns[r].size() << " patterns:" << std::endl;
    
        for(const auto& pattern : puzzle.rowPatterns[r]) {
            std::cerr << " ";
            for(int cell : pattern) {
                if(cell == FILLED) {
                    std::cerr << "#";
                } else {
                    std::cerr << ".";
                }
            }
            std::cerr << std::endl;
        }
    }

    std::cerr << std::endl;
    
    // Columns
    puzzle.colPatterns.resize(puzzle.cols);
    for (int c = 0; c < puzzle.cols; c++) {
        puzzle.colPatterns[c] = generatePatterns(puzzle.rows, puzzle.colClues[c]);
        std::cerr << "Col " << c << " clues: ";
        for (int r : puzzle.colClues[c]) std::cerr << r << " ";
        std::cerr << "-> " << puzzle.colPatterns[c].size() << " patterns:" << std::endl;
        for (const auto& p : puzzle.colPatterns[c]) {
            std::cerr << "  ";
            for (int cell : p) {
                if (cell == FILLED) std::cerr << "#";
                else std::cerr << ".";
            }
            std::cerr << std::endl;
        }
    }

}

void testBitmask() {
    std::cerr << "Testing bitmask conversion..." << std::endl;
    std::vector<int> pattern = {UNKNOWN, FILLED, UNKNOWN, FILLED, UNKNOWN, FILLED, FILLED, UNKNOWN, FILLED};
    uint32_t mask = patternToMask(pattern);
    
    std::cerr << "Pattern: ";
    for (int cell : pattern) {
        std::cerr << (cell == FILLED ? "#" : ".");
    }
    std::cerr << std::endl;
    std::cerr << "Bitmask: " << mask << std::endl;
}

void testMasks() {
    std::cerr << "Testing AND/OR mask computation..." << std::endl;
    std::cerr << "Patterns for {3} in 5 cells:" << std::endl;

    auto patterns = generatePatterns(8, {5});

    std::vector<uint32_t> masks;
    for (const auto& p : patterns) {
        uint32_t m = patternToMask(p);
        masks.push_back(m);

        std::cerr << "Pattern: ";
        for (int cell : p) std::cerr << (cell == FILLED ? "#" : ".");
        std::cerr << " -> Mask: " << m << " BINARY: ";
        for (int i = 0; i < 9; i ++) std::cerr << ((m >> i) & 1);
        std::cerr << ")" <<std::endl;
    }

    uint32_t andMask = computeAndMask(masks);
    uint32_t orMask = computeOrMask(masks);

    std::cerr << "AND Mask: " << andMask << " BINARY: ";
    for (int i = 0; i < 9; i ++) std::cerr << ((andMask >> i) & 1);
    std::cerr << std::endl;
    std::cerr << "OR Mask: " << orMask << " BINARY: ";
    for (int i = 0; i < 9; i ++) std::cerr << ((orMask >> i) & 1);
    std::cerr << std::endl;
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

void testInitializeDomains() {
    Puzzle puzzle;
    puzzle.rows = 5;
    puzzle.cols = 5;
    puzzle.rowClues = {
        {3},
        {1, 3},
        {5},
        {1, 1},
        {3}
    };
    puzzle.colClues = {
        {3},
        {1, 1},
        {5},
        {2, 2},
        {3}
    };
    puzzle.board = std::vector<std::vector<int>>(puzzle.rows, std::vector<int>(puzzle.cols, UNKNOWN));

    // generate patterns
    puzzle.rowPatterns.resize(puzzle.rows);
    puzzle.rowPatternMasks.resize(puzzle.rows);
    for(int r = 0; r < puzzle.rows; r++) {
        puzzle.rowPatterns[r] = generatePatterns(puzzle.cols, puzzle.rowClues[r]);
        puzzle.rowPatternMasks[r].resize(puzzle.rowPatterns[r].size());
        puzzle.rowPatternMasks[r].clear();
        for( const auto& p : puzzle.rowPatterns[r]) {
            puzzle.rowPatternMasks[r].push_back(patternToMask(p));
        }
    }

    puzzle.colPatterns.resize(puzzle.cols);
    puzzle.colPatternMasks.resize(puzzle.cols);
    for (int c = 0; c < puzzle.cols; c++ ) {
        puzzle.colPatterns[c] = generatePatterns(puzzle.rows, puzzle.colClues[c]);
        puzzle.colPatternMasks[c].resize(puzzle.colPatterns[c].size());
        puzzle.colPatternMasks[c].clear();
        for( const auto& p : puzzle.colPatterns[c]) {
            puzzle.colPatternMasks[c].push_back(patternToMask(p));
        }
    }

    // Intialize domains
    initializeDomains(puzzle);

    // Print Domains
    std::cerr << "Initial Domains" << std::endl;
    for (int r = 0; r < puzzle.rows; r++) {
        std::cerr   << "Row " << r
                    << " | patterns: "
                    << puzzle.rowDomains[r].patternIndeces.size()
                    << " | AND = ";

        for(int i = 0; i < puzzle.cols; i++) {
            std::cerr << ((puzzle.rowDomains[r].andMask >> i) & 1);
        }
        std::cerr << " | OR = ";
        for(int i = 0; i < puzzle.cols; i++) {
            std::cerr << ((puzzle.rowDomains[r].orMask >> i) & 1);
        }
        std::cerr << std::endl;
    }

    for (int c = 0; c < puzzle.cols; c++) {
        std::cerr   << "Col " << c
                    << " | patterns: "
                    << puzzle.colDomains[c].patternIndeces.size()
                    << " | AND = ";

        for(int i = 0; i < puzzle.rows; i++) {
            std::cerr << ((puzzle.colDomains[c].andMask >> i) & 1);
        }
        std::cerr << " | OR = ";
        for(int i = 0; i < puzzle.rows; i++) {
            std::cerr << ((puzzle.colDomains[c].orMask >> i) & 1);
        }
        std::cerr << std::endl;
    }

    std::cerr << "\n=== Direct call: generatePatterns(5, {1,1}) ===" << std::endl;
    auto directPatterns = generatePatterns(5, {1, 1});
    std::cerr << "Count: " << directPatterns.size() << std::endl;
    for (const auto& p : directPatterns) {
        std::cerr << "  ";
        for (int cell : p) std::cerr << (cell == FILLED ? "#" : ".");
        std::cerr << std::endl;
    }

    // Now what's stored in puzzle.rowPatterns[3]
    std::cerr << "\n=== puzzle.rowPatterns[3] (" << puzzle.rowPatterns[3].size() << " entries) ===" << std::endl;
    for (int i = 0; i < puzzle.rowPatterns[3].size(); i++) {
        std::cerr << "  [" << i << "] ";
        for (int cell : puzzle.rowPatterns[3][i]) std::cerr << (cell == FILLED ? "#" : ".");
        std::cerr << std::endl;
    }

    // And colPatterns[1]
    std::cerr << "\n=== puzzle.colPatterns[1] (" << puzzle.colPatterns[1].size() << " entries) ===" << std::endl;
    for (int i = 0; i < puzzle.colPatterns[1].size(); i++) {
        std::cerr << "  [" << i << "] ";
        for (int cell : puzzle.colPatterns[1][i]) std::cerr << (cell == FILLED ? "#" : ".");
        std::cerr << std::endl;
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

Puzzle testSingleFilter(Puzzle& puzzle) {
    puzzle.board = std::vector<std::vector<int>>(puzzle.rows, std::vector<int>(puzzle.cols, UNKNOWN));
    // generate patterns
    puzzle.rowPatterns.resize(puzzle.rows);
    puzzle.rowPatternMasks.resize(puzzle.rows);
    for(int r = 0; r < puzzle.rows; r++) {
        puzzle.rowPatterns[r] = generatePatterns(puzzle.cols, puzzle.rowClues[r]);
        std::cerr << "Row " << r << " patterns: " << puzzle.rowPatterns[r].size() << std::endl;
        
        for(int c : puzzle.rowClues[r]) {
            std::cerr << c << " ";
        }
        std::cerr << "-> " << puzzle.rowPatterns[r].size() << " patterns:" << std::endl;
    
        for(const auto& pattern : puzzle.rowPatterns[r]) {
            std::cerr << " ";
            for(int cell : pattern) {
                if(cell == FILLED) {
                    std::cerr << "#";
                } else {
                    std::cerr << ".";
                }
            }
            std::cerr << std::endl;
        }

        puzzle.rowPatternMasks[r].clear();
        for(const auto& p : puzzle.rowPatterns[r]) {
            puzzle.rowPatternMasks[r].push_back(patternToMask(p));
        }
    }

    puzzle.colPatterns.resize(puzzle.cols);
    puzzle.colPatternMasks.resize(puzzle.cols);
    for(int c = 0; c < puzzle.cols; c++) {
        puzzle.colPatterns[c] = generatePatterns(puzzle.rows, puzzle.colClues[c]);
        puzzle.colPatternMasks[c].clear();
        for(const auto& p : puzzle.colPatterns[c]) {
            puzzle.colPatternMasks[c].push_back(patternToMask(p));
        }
    }

    initializeDomains(puzzle);

    std::cerr << "====== before filtering ========" << std::endl;
    std::cerr << "Col 4 AND = ";
    for(int r = 0; r < puzzle.rows; r++)
        std::cerr << ((puzzle.colDomains[4].andMask >> r) & 1);
    std::cerr << std::endl;

    std::cerr << "Row 2 domain size before: " << puzzle.rowDomains[2].patternIndeces.size() << std::endl;

    int testRow = 2;
    std::cerr << "\nColumn constraints for row " << testRow << ":" << std::endl;
    for(int c = 0; c < puzzle.cols; c++) {
        bool guaranteeFilled = (puzzle.colDomains[c].andMask >> testRow) & 1;
        bool guaranteeEmpty = !((puzzle.colDomains[c].orMask >> testRow) & 1);
        std::cerr << "Col " << c
                    << " AND bit " << testRow << " = " << guaranteeFilled
                    << " OR bit " << testRow << " = "
                    << ((puzzle.colDomains[c].orMask >> testRow) & 1)
                    << " guaranteed empty = " << guaranteeEmpty << ")" << std::endl;
    }

    filterRowPatterns(puzzle, 2);

    std::cerr << "Row 2 domain size after " << puzzle.rowDomains[2].patternIndeces.size() << std::endl;

    std::cerr << "Row 2 remaining patterns: " << std::endl;
    for(int i : puzzle.rowDomains[2].patternIndeces) {
        std::cerr << " ";
        for(int cell : puzzle.rowPatterns[2][i])
            std::cerr << (cell == FILLED ? "#" : ".");
        std::cerr << std::endl;
    }

    return puzzle;
    
}


int main(int argc, char* argv[]) {
    std::string action = argv[1];
    int rows = std::stoi(argv[2]);
    int cols = std::stoi(argv[3]);

    Puzzle puzzle = readPuzzle(rows, cols);
    // Puzzle puzzle;
    if(action == "solve") {
        // solvePuzzle(puzzle);
        // testGenerateAllPatterns();
        // testBitmask();
        // testMasks();
        // testInitializeDomains();
        puzzle = testSingleFilter(puzzle);
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return 1;
    }

    printBoard(puzzle);
    
    return 0;
}

