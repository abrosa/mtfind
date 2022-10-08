/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <array>
#include <chrono>
#include <ctime>

static const uintmax_t MAX_THREADS = 8;

class Block {
public:
    Block(uintmax_t n, uintmax_t l, char* begin, uintmax_t size, char* mask, uintmax_t mask_size, uintmax_t total_lines) {
        this->n = n;
        this->l = l;
        this->begin = begin;
        this->size = size;
        this->mask = mask;
        this->mask_size = mask_size;
        this->total_lines = total_lines;
    }
    uintmax_t n;  // number of block
    uintmax_t l;  // number of line in block
    char* begin;
    uintmax_t size;
    char* mask;
    uintmax_t mask_size;
    uintmax_t total_lines;
};

class Result {
public:
    Result(uintmax_t block, uintmax_t line, uintmax_t position, std::string found) {
        this->block = block;
        this->line = line;
        this->position = position;
        this->found = found;
    }
    uintmax_t block;
    uintmax_t line;
    uintmax_t position;
    std::string found;
};

inline bool compare_mask(char* line, char* mask, uintmax_t mask_size) {
    uintmax_t i = 0;
    for (; line && mask && i != mask_size; ++line, ++mask, ++i) {
        if (*line == '\n' || *mask != '?' && *mask != *line) return false;
    }
    if (i == mask_size) return true;
    return false;
}

void process_line(Block & line, std::vector <Result> & results) {
    char* begin = line.begin;
    uintmax_t len = line.size;
    while (line.begin && len + begin - line.begin >= line.mask_size) {
        if (compare_mask(line.begin, line.mask, line.mask_size)) {
            std::string found(line.begin, line.mask_size);
            Result current(line.n, line.l, 1l + line.begin - begin, found);
            results.push_back(current);
        }
        ++line.begin;
    }
}

void process_block(Block & block, std::vector <Result> & results) {
    Result empty(block.n, 0, 0, "");
    results.push_back(empty);
    char* begin = block.begin;
    uintmax_t len = block.size;
    while (block.begin && len > 0) {
        if (*block.begin == '\n') {
            Block curr(block.n, block.total_lines, begin, block.begin - begin + 1, block.mask, block.mask_size, 1);
            process_line(curr, results);
            len -= block.begin + 1 - begin;
            begin = block.begin + 1;
            ++block.total_lines;
        }
        ++block.begin;
    }
}

void merge_results(std::vector <Block>& blocks, std::vector <Result>& results) {
    uintmax_t total_lines = 1;
    for (auto& block : blocks) {
        for (auto& result : results) {
            if (result.block == block.n) {
                result.line += total_lines;
            }
        }
        total_lines += block.total_lines;
    }
    uintmax_t total_found = 0;
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
    uintmax_t leftover = block.size;
    uintmax_t block_size = leftover / MAX_THREADS;
    while (block.begin && block.size > 0) {
        ++block.n;
        if (block.n > MAX_THREADS) {
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
    std::cout << "process started at " << start_time << std::endl;

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
    uintmax_t mask_size = search_mask.length();
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
    std::cout << "process finished at " << end_time << std::endl;

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "elapsed seconds: " << elapsed_seconds.count() << "s" << std::endl;
}
