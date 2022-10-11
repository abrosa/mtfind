﻿/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

static const uint64_t MAX_THREADS = 8;

static const uint64_t MAX_FILE_SIZE = 1024 * 1024 * 1024;  // 1 GB

static const uint64_t MAX_MASK_LENGTH = 100;

static char mask_str[MAX_MASK_LENGTH];

static uint64_t mask_len;

class Block {
public:
    Block(uint64_t number, uint64_t lines, char* begin, char* end) {
        this->number = number;
        this->lines = lines;
        this->begin = begin;
        this->end = end;
    }
    uint64_t number;
    uint64_t lines;
    char* begin;
    char* end;
};

class Result {
public:
    Result(uint64_t block, uint64_t line, uint64_t position, std::string found) {
        this->block = block;
        this->line = line;
        this->position = position;
        this->found = found;
    }
    uint64_t block;
    uint64_t line;
    uint64_t position;
    std::string found;
};

void process_block(Block & block, std::vector <Result> & results) {
    char* i = block.begin;
    char* j;
    uint64_t k;
    for (char* block_begin = block.begin; block_begin < block.end; ++block_begin) {
        if (*block_begin != '\n') {
            continue;
        }
        for (j = i; j < block_begin - mask_len; ++j) {
            for (k = 0; k < mask_len; ++k) {
                if (!(j + k) || *(j + k) == '\n') break;
                if (!mask_str[k] || mask_str[k] == '\0') break;
                if (mask_str[k] != '?' && mask_str[k] != *(j + k)) break;
            }
            if (k != mask_len) {
                continue;
            }
            std::string found(j, mask_len);
            Result current(block.number, block.lines, j - i + 1, found);
            results.push_back(current);
        }
        ++block.lines;
        i = block_begin + 1;
        if (!i || i > block.end) {
            break;
        }
    }
}

void merge_results(std::vector <Block>& blocks, std::vector <Result>& results) {
    uint64_t total_lines = 0;
    for (auto& block : blocks) {
        for (auto& result : results) {
            if (result.block == block.number) {
                result.line += total_lines;
            }
        }
        total_lines += block.lines;
    }
    uint64_t total_found = 0;
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

void split_to_blocks(char* file_begin, uint64_t file_size, std::vector<Block>& blocks) {
    char* block_begin = file_begin;
    char* block_end;
    uint64_t block_number = 0;
    uint64_t block_lines = 1;
    uint64_t block_size = file_size / MAX_THREADS;
    while (block_begin && block_size > 0) {
        block_size = std::min(block_size, file_size);
        block_end = block_begin + block_size - 1;
        if (!block_end) {
            break;
        }
        while (block_end && *block_end != '\n') {
            ++block_end;
            ++block_size;
        }
        Block new_block(block_number, block_lines, block_begin, block_end);
        blocks.push_back(new_block);
        block_begin += block_size;
        file_size -= block_size;
        if (!block_begin) {
            break;
        }
        ++block_number;
        if (block_number >= MAX_THREADS) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    auto start_time = std::chrono::system_clock::now();
    std::cout << "process started at " << start_time << std::endl;
    std::string file_name;
    std::string search_mask;
    if (argc < 3) {
        file_name = "./resources/hugetext.bin";
        search_mask = "n?gger";
    }
    else {
        file_name = argv[1];
        search_mask = argv[2];
    }

    mask_len = search_mask.length();
    strcpy_s(mask_str, mask_len + 1, search_mask.c_str());

    boost::iostreams::mapped_file file;
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        std::vector<Block> blocks;
        split_to_blocks((char*)file.const_data(), file.size(), blocks);
        std::vector <std::thread> threads;
        std::vector <Result> results;
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

    auto end_time = std::chrono::system_clock::now();
    std::cout << "process finished at " << end_time << std::endl;

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "elapsed seconds: " << elapsed_seconds.count() << "s" << std::endl;
}
