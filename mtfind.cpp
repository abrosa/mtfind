/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>
#include "mtfind.h"

typedef std::array <int, 3> t_result;
typedef std::vector <t_result> t_results;

static const size_t BLOCK_SIZE = 65536;

// Ulysses.txt ~ 23,6 blocks

class Block {
public:
    Block(int n, char* begin, char* end) {
        this->n = n;
        this->begin = begin;
        this->end = end;
    }
    int n;
    char* begin;
    char* end;
};

void d_count(Block block, t_results & results) {
    t_result result = { 1, 0, 1 };
    while (block.begin && block.begin < block.end) {
        if (*block.begin == '\n') {
            ++result[0];
        }
        if (*block.begin == 'd') {
            ++result[1];
        }
        ++block.begin;
        ++result[2];
    }
    results[block.n] = result;
}

void print_results(t_results & results) {
    t_result total = { 0, 0, 0 };
    int i = 0;
    for (auto & result : results) {
        if (result[0] == 0) {
            break;
        }
        std::cout << "  Block: " << std::format("{:4}", i);
        std::cout << ", Lines: " << result[0];
        std::cout << ", 'd'ch: " << result[1];
        std::cout << ", Bytes: " << result[2] << std::endl;
        total[0] += result[0];
        total[1] += result[1];
        total[2] += result[2];
        ++i;
    }
    std::cout << "Total: Blocks: " << std::format("{:4}", i);
    std::cout << ", Lines: " << total[0];
    std::cout << ", 'd'ch: " << total[1];
    std::cout << ", Bytes: " << total[2] << std::endl;
}

int main() {
    std::vector<std::thread> threads;

    std::vector<Block> blocks;

    boost::iostreams::mapped_file file;

    std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    // std::string file_name = "dabadee.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();
    size_t max_blocks = 1 + file_size / BLOCK_SIZE;
    t_results results(max_blocks, { 0, 0, 0 });
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
            Block block(i, bob, eob);
            blocks.push_back(block);
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
        for (auto& block : blocks) {
            threads.push_back(std::thread(d_count, block, std::ref(results)));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        print_results(results);

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
