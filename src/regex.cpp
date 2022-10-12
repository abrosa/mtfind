/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <ctime>
#include <string>
#include <algorithm>

namespace regex {
    void process_data(std::string file_name, std::string mask_str) {
        auto now1 = std::chrono::system_clock::now();
        auto time1 = std::chrono::system_clock::to_time_t(now1);
        //cout << ctime(&time1) << endl;

        std::ifstream in(file_name, std::ifstream::ate | std::ifstream::binary);
        uint64_t file_size = in.tellg();
        uint64_t mask_len = mask_str.length();

        std::stringstream result;
        
        std::ifstream input(file_name);
        int lines = 1;
        int matches = 0;

        std::smatch res;
        std::regex exp(mask_str);

        for (std::string line; getline(input, line); ++lines) {
            uint64_t pos = 0;
            while (regex_search(line.cbegin() + pos, line.cend(), res, exp)) {
                pos += res.position();
                result << lines << " " << pos + 1 << " " << res[0] << std::endl;
                pos += res.length();
                ++matches;
            }
        }
        std::cout << matches << std::endl;
        std::cout << result.str();

        auto now2 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::to_time_t(now2);
        //cout << ctime(&time2) << endl;

        std::chrono::duration<double> elapsed_seconds = now2 - now1;
        std::cout << "regex: " << elapsed_seconds.count() << "s" << std::endl;
    }
}
