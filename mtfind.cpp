/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>
#include "mtfind.h"

typedef std::array <uintmax_t, 3> t_result;
typedef std::vector <t_result> t_results;

static const size_t BLOCK_SIZE = 65536;

// Ulysses.txt ~ 23,6 blocks

class Block {
public:
    Block(int n, char* begin, size_t size) {
        this->n = n;
        this->begin = begin;
        this->size = size;
    }
    int n;
    char* begin;
    size_t size;
};

void d_count(Block block, t_results & results) {
    size_t size = block.size;
    t_result result = { 0, 0, 0 };
    while (block.begin && size > 0) {
        if (*block.begin == '\n') {
            ++result[0];
        }
        if (*block.begin == 'd') {
            ++result[1];
        }
        ++block.begin;
        --size;
        ++result[2];
    }
    results[block.n] = result;
}

void print_results(t_results & results) {
    t_result total = { 1, 0, 0 };
    int i = 0;
    for (auto & result : results) {
        // std::cout << "  Block: " << std::format("{:4}", i);
        // std::cout << ", Lines: " << result[0];
        // std::cout << ", 'd'ch: " << result[1];
        // std::cout << ", Bytes: " << result[2] << std::endl;
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

void split_to_blocks(Block& block, std::vector<Block>& blocks)
{
    // int counter = 0;
    size_t leftover = block.size;
    size_t max_blocks = 1 + leftover / BLOCK_SIZE;
    while (block.begin && block.size > 0) {
        ++block.n;
        if (block.n > max_blocks) {
            std::cout << "block.n > max_blocks" << std::endl;
            break;
        }
        block.size = std::min(BLOCK_SIZE, leftover);
        leftover -= block.size;
        if (block.size <= 0) {
            // this one is a good break
            // std::cout << "block.size <= 0" << std::endl;
            break;
        }
        while (block.begin + block.size - 1 &&
            *(block.begin + block.size - 1) != '\n' && leftover > 0) {
            ++block.size;
            --leftover;
        }

        // std::cout << " block.n = " << block.n;
        // std::cout << " block.size = " << block.size;
        // counter += block.size;
        // std::cout << " total = " << counter;
        // std::cout << " leftover = " << leftover;
        // std::cout << std::endl;

        blocks.push_back(block);
        block.begin += block.size;
        if (!block.begin) {
            std::cout << "!block.begin" << std::endl;
            break;
        }
    }
}

int main() {
    boost::iostreams::mapped_file file;
    // std::string file_name = "test.bin";
    std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    // std::string file_name = "dabadee.txt";
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        Block block(-1, (char*)file.const_data(), file.size());
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector<std::thread> threads;
        t_results results(blocks.size(), {0, 0, 0});
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
