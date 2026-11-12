#ifndef PUZZLE_H
#define PUZZLE_H

#include <vector>
#include <cstdint>

constexpr int UNKNOWN = 0;
constexpr int FILLED = 1;
constexpr int CROSSED = -1;

struct Domain {
    std::vector<int> patternIndeces;
    uint32_t andMask;
    uint32_t orMask;
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



#endif