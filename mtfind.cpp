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

static const size_t BLOCK_SIZE = 16;

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

class Result {
public:
    int line;
    int position;
    std::string found;
};

void d_count(Block block, std::vector <std::vector <Result>> & results) {
    size_t size = block.size;
    std::vector <Result> res;
    int line = 1;
    int position = 1;
    while (block.begin && size > 0) {
        if (*block.begin == '\n') {
            ++line;
            position = 0;
        }
        if (*block.begin == 'd') {
            std::string found = "d";
            Result current;
            current.line = line;
            current.position = position;
            current.found = found;
            res.push_back(current);
        }
        ++block.begin;
        --size;
        ++position;
    }
    int i = 0;
    for (auto & result : results) {
        if (i == block.n) {
            result = res;
        }
        ++i;
    }
}

void print_results(std::vector <std::vector <Result>> & results) {
    int i = 0;
    for (auto & result : results) {
        for (auto& res : result) {
            std::cout << res.line;
            std::cout << " " << res.position;
            std::cout << " " << res.found << std::endl;
        }
        ++i;
    }
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
    std::string file_name = "task.txt";
    // std::string file_name = "test.bin";
    // std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    // std::string file_name = "dabadee.txt";
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        Block block(-1, (char*)file.const_data(), file.size());
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector<std::thread> threads;
        Result empty;
        empty.line = 0;
        empty.position = 0;
        empty.found = "";
        std::vector <Result> emptys;
        emptys.push_back(empty);
        std::vector <std::vector <Result>> results(blocks.size(), emptys);
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
