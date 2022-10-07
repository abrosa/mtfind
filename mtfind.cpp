/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>

// static const size_t MAX_THREADS = 8;
static const size_t BLOCK_SIZE = 65536;

// Ulysses.txt ~ 23,6 blocks * 65536

class Block {
public:
    Block(int n, char* begin, size_t size, std::string mask) {
        this->n = n;
        this->begin = begin;
        this->size = size;
        this->mask = mask;
    }
    int n;
    char* begin;
    size_t size;
    std::string mask;
};

class Result {
public:
    Result(int line, int position, std::string found) {
        this->line = line;
        this->position = position;
        this->found = found;
    }
    int line;
    int position;
    std::string found;
};

class BlockResults {
public:
    int total_lines;
    int total_bytes;
    int total_found;
    std::vector <Result> results;
};

bool compare_mask(char* line, std::string mask) {
    int len = mask.size();
    int pos = 0;
    while (line && *line != '\n' && pos < len) {
        if (mask[pos] != '?' && mask[pos] != *line) {
            return false;
        }
        ++pos;
        ++line;
    }
    if (pos == len) return true;
    return false;
}

void d_count(Block block, std::vector <BlockResults> & results) {
    BlockResults res;
    res.total_bytes = block.size;
    res.total_lines = 1;
    res.total_found = 0;
    int position = 1;
    while (block.begin && block.size > 0) {
        if (*block.begin == '\n') {
            ++res.total_lines;
            position = 0;
        }
        if (compare_mask(block.begin, block.mask)) {
            ++res.total_found;
            std::string found(block.begin, block.mask.size());
            Result current(res.total_lines, position, found);
            res.results.push_back(current);
        }
        ++block.begin;
        --block.size;
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

void print_results(std::vector <BlockResults> & results) {
    int i = 0;
    int total_lines = - 1;
    int total_bytes = 0;
    int total_found = 0;
    for (auto& result : results) {
        total_lines += result.total_lines - 1;
        total_bytes += result.total_bytes;
        total_found += result.total_found;
        std::cout << " total_bytes " << total_bytes;
        std::cout << " total_lines " << total_lines;
        std::cout << " total_found " << total_found << std::endl;
        for (auto& res : result.results) {
            std::cout << " " << result.total_lines + res.line;
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
    std::string search_mask = "nothing";
    // std::string file_name = "task.txt";
    // std::string file_name = "test.bin";
    std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    // std::string file_name = "dabadee.txt";
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        Block block(-1, (char*)file.const_data(), file.size(), search_mask);
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector<std::thread> threads;
        Result empty(0, 0, "");
        BlockResults emptys;
        emptys.results.push_back(empty);
        std::vector <BlockResults> results(blocks.size(), emptys);
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
