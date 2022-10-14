/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

// #include <../include/mtfind.hpp>

#include <iostream>
#include <string.h>

static const int CHARS_IN_X64 = 8;

static const int MAX_MASK_LENGTH = 101;

// '?' == 0x3F == 63
static const uint64_t qq = 63ull;
static const uint64_t q[CHARS_IN_X64] = {
    qq, (qq<<8), (qq<<16), (qq<<24), (qq<<32), (qq<<40), (qq<<48), (qq<<56)
};

// '\n' == 0x0A == 10
static const uint64_t nn = 10ull;
static const uint64_t n[CHARS_IN_X64] = {
    nn, (nn<<8), (nn<<16), (nn<<24), (nn<<32), (nn<<40), (nn<<48), (nn<<56)
};

// mask == 0xFF == 255
static const uint64_t ff = 255ull;
static const uint64_t f[CHARS_IN_X64] = {
    ff, (ff<<8), (ff<<16), (ff<<24), (ff<<32), (ff<<40), (ff<<48), (ff<<56)
};

inline const char * const BoolToString(bool b) {
  return b ? "true" : "false";
}

namespace compare_mask {
    void apply_it(uint64_t & l, uint64_t & m, uint64_t & mask_len);
    bool compare_mask(char *line_buf, char *mask_buf, uint64_t mask_len);
    bool compare_strings(std::string line_str, std::string mask_str);
    int main();
}
