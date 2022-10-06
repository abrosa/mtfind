/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>
#include "mtfind.h"

static const size_t BLOCK_SIZE = 64;

// Ulysses.txt ~ 23,6 blocks

void d_count(const int n, char* bob, char* eob, std::vector <std::array <int, 3>> & results) {
    int lines = 1;
    int d_cha = 0;
    int bytes = 1;
    while (bob && bob < eob) {
        if (*bob == 'd') {
            ++d_cha;
        }
        if (*bob == '\n') {
            ++lines;
        }
        ++bob;
        ++bytes;
    }
    results[n][0] = bytes;
    results[n][1] = d_cha;
    results[n][2] = lines;
}

void print_results(std::vector <std::array <int, 3>> & results)
{
    int total_bytes = 0;
    int total_d_cha = 0;
    int total_lines = 0;
    int i = 0;
    for (auto & result : results) {
        if (result[0] == 0) {
            break;
        }
        std::cout << "Block: " << std::format("{:6}", i);
        std::cout << ", Bytes: " << result[0];
        std::cout << ", 'd'ch: " << result[1];
        std::cout << ", Lines: " << result[2] << std::endl;
        total_bytes += result[0];
        total_d_cha += result[1];
        total_lines += result[2];
        ++i;
    }
    std::cout << "Total Bytes: " << total_bytes << std::endl;
    std::cout << "Total 'd'ch: " << total_d_cha << std::endl;
    std::cout << "Total Lines: " << total_lines << std::endl;
}

int main() {
    std::vector<std::thread> threads;


    boost::iostreams::mapped_file file;

    // std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    std::string file_name = "dabadee.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();
    size_t max_blocks = 1 + file_size / BLOCK_SIZE;
    std::vector <std::array <int, 3>> results(max_blocks, { 0, 0, 0 });
    size_t length = BLOCK_SIZE;

    if (file.is_open()) {
        char* data = (char*) file.const_data();
        char* bod = data;
        char* eod = data + file_size - 1;
        int i = 0;
        size_t position = 0;
        length = std::min(BLOCK_SIZE, file_size - position);
        char* bob = bod;
        char* eob = bob + length - 1;
        while (bob && eob) {
            while (eob && *eob != '\n') {
                ++eob;
            }
            if (!eob) {
                break;
            }
            threads.push_back(std::thread(d_count, i, bob, eob, std::ref(results)));
            position += eob - bob + 1;
            if (position >= file_size) {
                break;
            }
            bob = eob + 1;
            if (!bob) {
                break;
            }
            length = std::min(BLOCK_SIZE, file_size - position);
            eob = bob + length - 1;
            if (!eob) {
                break;
            }
            ++i;
        }

        for (auto& th : threads) {
            th.join();
        }

        print_results(results);

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
