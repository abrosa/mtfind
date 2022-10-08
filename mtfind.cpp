/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>

// static const size_t MAX_THREADS = 8;
static const size_t BLOCK_SIZE = 16;

// Ulysses.txt ~ 23,6 blocks * 65536

class Block {
public:
    Block(int n, char* begin, size_t size, std::string mask, int total_lines) {
        this->n = n;
        this->begin = begin;
        this->size = size;
        this->mask = mask;
        this->total_lines = total_lines;
    }
    int n;
    char* begin;
    size_t size;
    std::string mask;
    int total_lines;
};

class Result {
public:
    Result(int block, int line, int position, std::string found) {
        this->block = block;
        this->line = line;
        this->position = position;
        this->found = found;
    }
    int block;
    int line;
    int position;
    std::string found;
};

bool compare_mask(char* line, std::string mask) {
    size_t len = mask.size();
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

void process_line(Block& line, int curr_line, std::vector <Result> & results)
{
    int curr_pos = 1;
    while (line.begin && line.size > 0) {
        if (compare_mask(line.begin, line.mask)) {
            std::string found(line.begin, line.mask.size());
            Result current(line.n, curr_line, curr_pos, found);
            results.push_back(current);
        }
        ++line.begin;
        --line.size;
        ++curr_pos;
    }
}

void process_block(Block block, std::vector <Result> & results) {
    int curr_line = 1;
    char* curr_begin = block.begin;
    size_t curr_pos = 1;
    while (block.begin && block.size > 0) {
        if (*block.begin == '\n') {
            Block curr(block.n, curr_begin, curr_pos, block.mask, 1);
            process_line(curr, curr_line, results);
            curr_line++;
            curr_begin = block.begin + 1;
            curr_pos = 0;
            ++block.total_lines;
        }
        ++block.begin;
        --block.size;
        ++curr_pos;
    }
}

void merge_results(std::vector <Block>& blocks, std::vector <Result>& results) {
    int total_lines = 0;
    for (auto& block : blocks) {
        for (auto& result : results) {
            if (result.block == block.n) {
                result.line += total_lines;
            }
        }
        total_lines += block.total_lines;
    }
    int total_found = 0;
    for (auto& result : results) {
        ++total_found;
    }
    std::cout << total_found << std::endl;
}

void print_results(std::vector <Result> & results) {
    for (auto& result : results) {
        std::cout << result.line << " ";
        std::cout << result.position << " ";
        std::cout << result.found << std::endl;
    }
}

void split_to_blocks(Block& block, std::vector<Block>& blocks)
{
    size_t leftover = block.size;
    size_t max_blocks = 1 + leftover / BLOCK_SIZE;
    while (block.begin && block.size > 0) {
        ++block.n;
        if (block.n > max_blocks) {
            break;
        }
        block.size = std::min(BLOCK_SIZE, leftover);
        leftover -= block.size;
        if (block.size <= 0) {
            break;
        }
        while (block.begin + block.size - 1 &&
            *(block.begin + block.size - 1) != '\n' && leftover > 0) {
            ++block.size;
            --leftover;
        }
        blocks.push_back(block);
        block.begin += block.size;
        if (!block.begin) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string file_name;
    std::string search_mask;
    if (argc < 3) {
        file_name = "task.txt";
        search_mask = "?ad";
    }
    else {
        file_name = argv[1];
        search_mask = argv[2];
    }

    boost::iostreams::mapped_file file;
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        Block block(-1, (char*)file.const_data(), file.size(), search_mask, 1);
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector <std::thread> threads;
        std::vector <Result> results;
        for (auto& block : blocks) {
            threads.push_back(std::thread(process_block, block, std::ref(results)));
        }
        for (auto& thread : threads) {
            thread.join();
        }
        merge_results(blocks, results);
        print_results(results);
        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
