#ifndef RANDOM_H
#define RANDOM_H

typedef unsigned long long U64;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number();

// generate 64-bit pseudo legal numbers
U64 get_random_U64_number();

// generate magic number candidate
U64 generate_magic_number();

#endif