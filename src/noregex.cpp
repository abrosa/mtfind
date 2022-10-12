/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <ctime>
#include <string>
#include <algorithm>

static const uint64_t MAX_FILE_SIZE = 1024*1024*1024;
static char buffer[MAX_FILE_SIZE];
static const uint64_t MAX_MASK_LENGTH = 101;
static char mask_str[MAX_MASK_LENGTH];

namespace noregex {
    void process_data(std::string file_name, std::string search_mask) {
        auto now1 = std::chrono::system_clock::now();
        auto time1 = std::chrono::system_clock::to_time_t(now1);
        //cout << ctime(&time1) << endl;

        std::ifstream in(file_name, std::ifstream::ate | std::ifstream::binary);
        uint64_t file_size = in.tellg();

        uint64_t mask_len = search_mask.length();
        strcpy_s(mask_str, mask_len + 1, search_mask.c_str());

        std::stringstream result;

        std::ifstream input(file_name);
        int lines = 1;
        int matches = 0;

        input.read(buffer, file_size);

        char* begin = buffer;
        char* end = buffer + file_size - 1;
        char* i = begin;
        char* j;
        uint64_t k;
        for (; begin <= end; ++begin) {
            if (!begin) {
                break;
            }
            if (*begin != '\n')
                continue;
            for (j = i; j <= begin; ++j) {
                if (!j) {
                    break;
                }
                for (k = 0; k < mask_len; ++k) {
                    if (!j[k] || j[k] == '\n') break;
                    if (!mask_str[k] || mask_str[k] == '\0') break;
                    if (mask_str[k] != '?' && mask_str[k] != j[k]) break;
                }
                if (k != mask_len)
                    continue;
                std::string found(j, mask_len);
                result << lines << " " << j - i + 1 << " " << found << std::endl;
                ++matches;
            }
            ++lines;
            i = begin + 1;
            if (i > end) {
                break;
            }
        }
        std::cout << matches << std::endl;
        std::cout << result.str();

        auto now2 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::to_time_t(now2);
        //cout << ctime(&time2) << endl;

        std::chrono::duration<double> elapsed_seconds = now2 - now1;
        std::cout << "no regex: " << elapsed_seconds.count() << "s" << std::endl;
    }
}
