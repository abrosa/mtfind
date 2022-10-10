﻿/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <fstream>

namespace bio = boost::iostreams;

using namespace std;

ifstream::pos_type filesize(const char* filename) {
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

static const uint64_t MAX_THREADS = 8;

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
    Result(uint64_t block, uint64_t line, uint64_t position, string found) {
        this->block = block;
        this->line = line;
        this->position = position;
        this->found = found;
    }
    uint64_t block;
    uint64_t line;
    uint64_t position;
    string found;
};

void process_line(Block & line, vector <Result> & results) {
    uint64_t k = 0;
    for (char* j = line.begin; j <= line.begin + line.size - line.compare ; ++j) {
        for (k = 0; k < line.compare; ++k) {
            if (!(j + k) || *(j + k) == '\n') break;
            if (*(line.mask + k) != '?' && *(line.mask + k) != *(j + k)) break;
            
        }
        if (k == line.compare) {
            string found(j, line.compare);
            Result current(line.number, line.lines, j - line.begin + 1, found);
            results.push_back(current);
        }
    }
}

void process_block(Block & block, vector <Result> & results) {
    Result empty(block.number, 0, 0, "");
    results.push_back(empty);
    char* b = block.begin;
    char* i = block.begin;
    char* j = block.begin;
    uint64_t k = 0;
    for (i = block.begin; i <= block.begin + block.size; ++i) {
        if (i && *i == '\n') {
            for (j = b; j <= i + 1 - block.compare; ++j) {
                for (k = 0; k < block.compare; ++k) {
                    if (!(j + k) || *(j + k) == '\n') break;
                    if (*(block.mask + k) != '?' && *(block.mask + k) != *(j + k)) break;
                }
                if (k == block.compare) {
                    string found(j, block.compare);
                    Result current(block.number, block.lines, j - b + 1, found);
                    results.push_back(current);
                }
            }
            ++block.lines;
            b = i + 1;
        }
    }
}

void merge_results(vector <Block>& blocks, vector <Result>& results) {
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
    cout << total_found << endl;
    for (auto& result : results) {
        if (result.position != 0) {
            cout << result.line << " ";
            cout << result.position << " ";
            cout << result.found << endl;
        }
    }
}

void split_to_blocks(Block& block, vector<Block>& blocks)
{
    uint64_t leftover = block.size;
    uint64_t block_size = leftover / MAX_THREADS;
    while (block.begin && block.size > 0) {
        ++block.number;
        if (block.number > MAX_THREADS) {
            break;
        }
        block.size = min(block_size, leftover);
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

    auto start_time = chrono::system_clock::now();
    //cout << "process started at " << start_time << endl;

    string file_name;
    string search_mask;
    if (argc < 3) {
        file_name = "./resources/test.bin";
        search_mask = "d?sire";
        //cout << "Usage info: mtfind.exe file.txt \"m?sk\"" << endl;
        //return -1;
    }
    else {
        file_name = argv[1];
        search_mask = argv[2];
    }

    const uint64_t mask_len = search_mask.length();
    char* mask = new char[mask_len + 1];
    strcpy_s(mask, mask_len + 1, search_mask.c_str());

    size_t file_size = filesize(file_name.c_str());
    
    //cout << file_size << endl;
    
    bio::mapped_file file;
    bio::mapped_file_params params;
    params.path = file_name;
    params.flags = bio::mapped_file::mapmode::readwrite;
    params.length = file_size + 1;
    file.open(params);
    if (file.is_open()) {
        char* data = (char*)file.const_data();
        data[file_size] = '\n';
        Block block(0, 0, data, file_size + 1, mask, mask_len);
        vector<Block> blocks;
        split_to_blocks(block, blocks);
        vector <thread> threads;
        vector <Result> results;
        //for (auto& block : blocks) {
        //    process_block(block, results);
        //}
        for (auto& block : blocks) {
           threads.push_back(thread(process_block, ref(block), ref(results)));
        }
        for (auto& thread : threads) {
            thread.join();
        }
        merge_results(blocks, results);
        file.close();
    }
    else {
        cout << "could not map the file " << file_name << endl;
    }

    delete[] mask;

    auto end_time = chrono::system_clock::now();
    //cout << "process finished at " << end_time << endl;

    chrono::duration<double> elapsed_seconds = end_time - start_time;
    cout << "elapsed seconds: " << elapsed_seconds.count() << "s" << endl;
}
