/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

// #include <../include/mtfind.hpp>

#include <iostream>
#include <string.h>


namespace compare_mask {
    void apply_it(uint64_t & l, uint64_t & m, uint64_t & mask_len);
    bool compare_mask(char *line_buf, char *mask_buf, uint64_t mask_len);
    bool compare_strings(std::string line_str, std::string mask_str);
    int main();
}
