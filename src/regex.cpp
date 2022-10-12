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
        std::ifstream input(file_name);
        uint64_t mask_len = mask_str.length();
        std::stringstream ss;
        int matches = 0;
        int lines = 1;
        uint64_t pos = 0;
        std::smatch res;
        std::regex exp(mask_str);
        for (std::string line; std::getline(input, line); ++lines) {
            if (std::regex_search(line, res, exp)) {
                for (size_t i = 0; i < res.size(); ++i) {
                    ss << lines << " ";
                    ss << res.position(i) + 1 << " ";
                    ss << res[i] << std::endl;
                }
                ++matches;
            }
        }
        std::cout << matches << std::endl;
        std::cout << ss.str();
        auto now2 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::to_time_t(now2);
        //cout << ctime(&time2) << endl;
        std::chrono::duration<double> elapsed_seconds = now2 - now1;
        std::cout << "std::regex: " << elapsed_seconds.count() << "s" << std::endl;
    }
}
