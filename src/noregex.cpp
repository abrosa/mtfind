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
        std::ifstream input;
        input.open(file_name, std::ifstream::ate | std::ifstream::binary);
        uint64_t file_size = input.tellg();
        input.seekg(0, std::ios::beg);
        input.read(buffer, file_size);
        uint64_t mask_len = search_mask.length();
        strcpy_s(mask_str, mask_len + 1, search_mask.c_str());
        std::stringstream ss;
        int lines = 1;
        int matches = 0;
        char* end = buffer + file_size - 1;
        char* i = buffer;
        char* j;
        uint64_t k;
        for (char* begin = buffer; begin && begin <= end; ++begin) {
            if (*begin != '\n')
                continue;
            for (j = i; j && j <= begin; ++j) {
                for (k = 0; k < mask_len; ++k) {
                    if (!j[k] || j[k] == '\n') break;
                    if (!mask_str[k] || mask_str[k] == '\0') break;
                    if (mask_str[k] != '?' && mask_str[k] != j[k]) break;
                }
                if (k != mask_len)
                    continue;
                std::string found(j, mask_len);
                ss << lines << " ";
                ss << j - i + 1 << " ";
                ss << found << std::endl;
                ++matches;
            }
            ++lines;
            i = begin + 1;
            if (i > end) {
                break;
            }
        }
        std::cout << matches << std::endl;
        std::cout << ss.str();
        auto now2 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::to_time_t(now2);
        //cout << ctime(&time2) << endl;
        std::chrono::duration<double> elapsed_seconds = now2 - now1;
        std::cout << "single thread: " << elapsed_seconds.count() << "s" << std::endl;
    }
}
