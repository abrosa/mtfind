/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <../include/compare_mask.hpp>
#include <iostream>
#include <string.h>

namespace compare_mask {
    void apply_it(uint64_t & l, uint64_t & m, uint64_t & mask_len) {
        for (int i = 0; i < CHARS_IN_X64; ++i) {
            if (mask_len > 0) {
                if ((l & f[i]) == n[i]) {
                    mask_len = 0;
                    break;
                }
                if ((m & f[i]) == q[i]) {
                    l &= ~f[i];
                    l |= q[i];
                }
                --mask_len;
            } else {
                l &= ~f[i];
            }
        }
    }

    bool compare_mask(char *line_buf, char *mask_buf, uint64_t mask_len) {
        uint64_t* line = (uint64_t*)line_buf;
        uint64_t* mask = (uint64_t*)mask_buf;
        bool result = true;
        while (line && *line != '\0' && mask && *mask != '\0' && mask_len > 0) {
            uint64_t l = *line;
            uint64_t m = *mask;
            apply_it(l, m, mask_len);
            ++line;
            ++mask;
            if (l != m) result = false;
        }
        return result;
    }

    bool compare_strings(std::string line_str, std::string mask_str) {
        bool result = false;
        char line_buf[MAX_MASK_LENGTH] = {0};
        char mask_buf[MAX_MASK_LENGTH] = {0};
        uint64_t line_len = line_str.length();
        uint64_t mask_len = mask_str.length();
        strcpy_s(line_buf, line_len + 1, line_str.c_str());
        strcpy_s(mask_buf, mask_len + 1, mask_str.c_str());
        result = compare_mask::compare_mask(line_buf, mask_buf, mask_len);
        return result;
    }

    int main() {
        std::string line_str = "N?G_A,^ fuck you, mister Spielberg!";
        std::string mask_str = "N?G?A,?";
        bool result = compare_mask::compare_strings(line_str, mask_str);
        std::cout << std::endl << BoolToString(result) << std::endl;
        return 0;
    }
}
