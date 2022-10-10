/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

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
static const uint64_t MAX_MASK_LEN = 100;

uint64_t mask_len;
char* mask_str;

class Block {
public:
    Block(uint64_t number, uint64_t lines, char* begin, char* end) {
        this->number = number;
        this->lines = lines;
        this->begin = begin;
        this->end = end;
    }
    
    uint64_t number;   // Number of block
    uint64_t lines;    // number of Lines in block
    char* begin;       // pointer to Begin
    char* end;         // pointer to End
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

void process_block(Block & block, vector <Result> & results) {
    Result empty(block.number, 0, 0, "");
    results.push_back(empty);
    char* b = block.begin;
    char* i;
    char* j;
    uint64_t k;
    for (i = block.begin; i <= block.end; ++i) {
        if (i && *i == '\n') {
            for (j = b; j <= i - mask_len; ++j) {
                k = 0;
                while (k < mask_len && (*(mask_str + k) == '?' || *(mask_str + k) == *(j + k))) {
                    ++k;
                }
                if (k == mask_len) {
                    string found(j, mask_len);
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
    uint64_t leftover = block.end - block.begin + 1;
    uint64_t block_size = leftover / MAX_THREADS;
    while (block.begin && block.end) {
        ++block.number;
        if (block.number > MAX_THREADS) {
            break;
        }
        block.end = block.begin + min(block_size, leftover) - 1;
        if (!block.end) {
            break;
        }
        leftover -= block.end - block.begin - 1;
        while (block.end && *block.end != '\n' && leftover > 0) {
            ++block.end;
            --leftover;
        }
        if (!block.end) {
            break;
        }
        blocks.push_back(block);
        block.begin = block.end + 1;
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
        search_mask = "n?gger";
        //cout << "Usage info: mtfind.exe file.txt \"m?sk\"" << endl;
        //return -1;
    }
    else {
        file_name = argv[1];
        search_mask = argv[2];
    }

    mask_len = search_mask.length();
    mask_str = new char[MAX_MASK_LEN + 1];
    strcpy_s(mask_str, mask_len + 1, search_mask.c_str());

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
        Block block(0, 0, data, data + file_size);
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

    delete[] mask_str;

    auto end_time = chrono::system_clock::now();
    //cout << "process finished at " << end_time << endl;

    chrono::duration<double> elapsed_seconds = end_time - start_time;
    cout << "elapsed seconds: " << elapsed_seconds.count() << "s" << endl;
}
