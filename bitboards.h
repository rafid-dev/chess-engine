#ifndef BITBOARDS_H
#define BITBOARDS_H

typedef unsigned long long U64;

// not A file constant
const U64 not_a_file = 18374403900871474942ULL;

// not H file constant
const U64 not_h_file = 9187201950435737471ULL;

// not HG file constant
const U64 not_hg_file = 4557430888798830399ULL;

// not AB file constant
const U64 not_ab_file = 18229723555195321596ULL;

// bishop relevant occupancy bit count for every square on board
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

#endif