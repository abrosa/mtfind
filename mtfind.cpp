/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

static const uint64_t MAX_THREADS = 8;
// my home computer have 4 CPU and theoretically 8 Threads
// but in practice
// without multithreading time = 16 sec
// with multithreading time = 4 sec


class Block {
public:
    Block(uint64_t number, uint64_t lines, char* begin, uint64_t size, char* mask, uint64_t compare) {
        this->number = number;
        this->lines = lines;
        this->begin = begin;
        this->size = size;
        this->mask = mask;
        this->compare = compare;
    }
    
    uint64_t number;   // Number of block
    uint64_t lines;    // number of Lines in block
    char* begin;       // pointer to Begin
    uint64_t size;     // Size of data
    char* mask;        // Mask to compare
    uint64_t compare;  // bytes to Compare
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

void process_line(Block & line, std::vector <Result> & results) {
    char* line_begin;
    char* line_mask;
    uint64_t line_compare;
    for (char* old_begin = line.begin; line.begin; ++line.begin) {
        line_begin = line.begin;
        line_mask = line.mask;
        line_compare = line.compare;
        while (line_begin && *line_begin != '\n' && (*line_mask == '?' || *line_mask == *line_begin)) {
            ++line_begin;
            ++line_mask;
            --line_compare;
        }
        if (line_compare == 0) {
            std::string found(line.begin, line.compare);
            Result current(line.number, line.lines, line.begin - old_begin + 1, found);
            results.push_back(current);
        }
        if (line.begin + line.compare > old_begin + line.size) {
            break;
        }
    }
}

void process_block(Block & block, std::vector <Result> & results) {
    Result empty(block.number, 0, 0, "");
    results.push_back(empty);
    for (char* block_begin = block.begin; block.begin; ++block.begin) {
        if (*block.begin == '\n') {
            Block curr(block.number, block.lines, block_begin, block.begin - block_begin + 1, block.mask, block.compare);
            process_line(curr, results);
            ++block.lines;
            block.size -= block.begin - block_begin + 1;
            block_begin = block.begin + 1;
            if (block.size <= 0) {
                break;
            }
        }
    }
}

void merge_results(std::vector <Block>& blocks, std::vector <Result>& results) {
    uint64_t total_lines = 1;
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

void split_to_blocks(Block& block, std::vector<Block>& blocks)
{
    uint64_t leftover = block.size;
    uint64_t block_size = leftover / MAX_THREADS;
    while (block.begin && block.size > 0) {
        ++block.number;
        if (block.number > MAX_THREADS) {
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

    const uint64_t mask_len = search_mask.length();
    char* mask = new char[mask_len + 1];
    strcpy_s(mask, mask_len + 1, search_mask.c_str());

    boost::iostreams::mapped_file file;
    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
    if (file.is_open()) {
        Block block(0, 0, (char*)file.const_data(), file.size(), mask, mask_len);
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector <std::thread> threads;
        std::vector <Result> results;
        //for (auto& block : blocks) {
        //    process_block(block, results);
        //}
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
