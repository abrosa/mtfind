/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>
#include <chrono>
#include <ctime>

static const size_t MAX_THREADS = 8;

class Block {
public:
    Block(int n, int l, char* begin, size_t size, char* mask, size_t mask_size, int total_lines) {
        this->n = n;
        this->l = l;
        this->begin = begin;
        this->size = size;
        this->mask = mask;
        this->mask_size = mask_size;
        this->total_lines = total_lines;
    }
    int n;  // number of block
    int l;  // number of line in block
    char* begin;
    size_t size;
    char* mask;
    size_t mask_size;
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

bool compare_mask(char* line, char* mask, size_t mask_size) {
    while (*line != '\n' && mask_size > 0) {
        if (*mask != '?' && *mask != *line) {
            break;
        }
        ++line;
        ++mask;
        --mask_size;
    }
    if (mask_size == 0) return true;
    return false;
}

void process_line(Block & line, std::vector <Result> & results) {
    // std::cout << "line (" << line.l << ")" << std::endl;
    int curr_pos = 1;
    while (line.begin && line.size >= line.mask_size) {
        if (compare_mask(line.begin, line.mask, line.mask_size)) {
            std::string found(line.begin, line.mask_size);
            Result current(line.n, line.l, curr_pos, found);
            results.push_back(current);
        }
        ++line.begin;
        --line.size;
        ++curr_pos;
    }
}

void process_block(Block & block, std::vector <Result> & results) {
    Result empty(block.n, 0, 0, "");
    results.push_back(empty);
    int curr_line = 0;
    char* curr_begin = block.begin;
    size_t curr_pos = 1;
    while (block.begin && block.size > 0) {
        if (*block.begin == '\n') {
            Block curr(block.n, curr_line, curr_begin, curr_pos, block.mask, block.mask_size, 1);
            process_line(curr, results);
            curr_line++;
            curr_begin = block.begin + 1;
            curr_pos = 0;
            ++block.total_lines;
        }
        ++block.begin;
        --block.size;
        ++curr_pos;
    }
    // std::cout << "    block (" << block.n << ") total (" << block.total_lines << ")" << std::endl;
}

void merge_results(std::vector <Block>& blocks, std::vector <Result>& results) {
    int total_lines = 1;
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
        if (result.position != 0) {
            ++total_found;
        }
    }
    std::cout << total_found << std::endl;
    for (auto& result : results) {
        if (result.position != 0) {
            std::cout << result.line << " ";
            std::cout << result.position << " ";
            std::cout << result.found << std::endl;
        }
    }
}

void split_to_blocks(Block& block, std::vector<Block>& blocks)
{
    size_t leftover = block.size;
    size_t max_blocks = MAX_THREADS;
    size_t block_size = leftover / MAX_THREADS;
    while (block.begin && block.size > 0) {
        ++block.n;
        if (block.n > max_blocks) {
            break;
        }
        block.size = std::min(block_size, leftover);
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
    auto start_time = std::chrono::system_clock::now();
    //std::cout << "process started at " << start_time << std::endl;

    std::string file_name;
    std::string search_mask;
    if (argc < 3) {
        file_name = "test.bin";
        search_mask = "n?gger";
    }
    else {
        file_name = argv[1];
        search_mask = argv[2];
    }
    // convert mask to pointer
    size_t mask_size = search_mask.length();
    char* mask = new char[mask_size + 1];
    strcpy_s(mask, mask_size + 1, search_mask.c_str());

    boost::iostreams::mapped_file file;
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        // std::cout << "block (" << 0 << ") line (" << 0 << ")" << std::endl;
        Block block(0, 0, (char*)file.const_data(), file.size(), mask, mask_size, 0);
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector <std::thread> threads;
        std::vector <Result> results;
        // without multithreading = 21s
        // for (auto& block : blocks) {
        //     process_block(block, results);
        // }
        // with multithreading = 7s
        for (auto& block : blocks) {
            threads.push_back(std::thread(process_block, std::ref(block), std::ref(results)));
        }
        for (auto& thread : threads) {
            thread.join();
        }
        merge_results(blocks, results);
        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }

    delete[] mask;

    auto end_time = std::chrono::system_clock::now();
    //std::cout << "process finished at " << end_time << std::endl;

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    //std::cout << "elapsed seconds: " << elapsed_seconds.count() << "s" << std::endl;
}
