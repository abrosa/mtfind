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
    BlockResults(int total_lines, size_t total_bytes, int total_found, std::vector<Result> results) {
        this->total_lines = total_lines;
        this->total_bytes = total_bytes;
        this->total_found = total_found;
        this->results = results;
    }
    int total_lines = 0;
    size_t total_bytes = 0;
    int total_found = 0;
    std::vector <Result> results;
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

void d_count(Block block, std::vector <BlockResults> & results) {
    Result empty(0, 0, "");
    BlockResults res(1, block.size, 0, {empty});
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

void merge_results(std::vector <BlockResults>& results) {
    int total_found = 0;
    for (auto& result : results) {
        total_found += result.total_found;
    }
    std::cout << total_found << std::endl;
}

void print_results(std::vector <BlockResults>& results) {
    int total_lines = -1;
    int total_found = 0;
    for (auto& result : results) {
        for (auto& res : result.results) {
            if (res.position != 0) {
                std::cout << total_lines + res.line + 1;
                std::cout << " " << res.position;
                std::cout << " " << res.found << std::endl;
            }
        }
        total_lines += result.total_lines - 1;
    }
}

void split_to_blocks(Block& block, std::vector<Block>& blocks)
{
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
            std::cout << "!block.begin" << std::endl;
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
        Block block(-1, (char*)file.const_data(), file.size(), search_mask);
        std::vector<Block> blocks;
        split_to_blocks(block, blocks);
        std::vector<std::thread> threads;
        Result empty(0, 0, "");
        BlockResults emptys(0, 0, 0, {empty});
        std::vector <BlockResults> results(blocks.size(), emptys);
        for (auto& block : blocks) {
            threads.push_back(std::thread(d_count, block, std::ref(results)));
        }
        for (auto& thread : threads) {
            thread.join();
        }
        merge_results(results);
        print_results(results);
        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
