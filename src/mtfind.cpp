﻿/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <../include/mtfind.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

static const uint64_t MAX_THREADS = 16;
static const uint64_t MAX_MASK_LENGTH = 100;
static char mask[MAX_MASK_LENGTH];
static uint64_t mlen;

namespace mtfind {
    void process_block(Block & block) {
        std::vector <Result> results;
        uint64_t line = 0;
        char* i = block.begin;
        char* j;
        uint64_t k = 0;
        char mc;
        char jc;
        for (char* c = block.begin; c && c <= block.end; ++c) {
            if (*c != '\n' && c != block.end) {
                continue;
            }
            j = i;
            while (j <= c - mlen + 1) {
                mc = *mask;
                jc = *j;
                while (j <= c - mlen + 1) {
                    if (mc == '?' || mc == jc) break;
                    jc = *j++;
                }
                k = 0;
                while (k < mlen) {
                    if (jc == '\n') break;
                    if (mc == '\0') break;
                    if (mc != '?' && mc != jc) break;
                    ++k;
                    mc = *(mask + k);
                    jc = *(j + k);
                }
                if (k != mlen || jc == '\n') {
                    ++j;
                    continue;
                }
                std::string found(j, mlen);
                Result current(line, j - i + 1, found);
                results.push_back(current);
                j += mlen;
            }
            ++line;
            i = c + 1;
            if (!i || i > block.end) {
                break;
            }
        }
        Result empty(line, 0, "");
        results.push_back(empty);
        block.results = results;
    }

    void merge_results(std::vector <Block>& blocks) {
        std::stringstream ss;
        uint64_t total_lines = 1;
        uint64_t total_found = 0;
        for (auto& block : blocks) {
            for (auto& result : block.results) {
                if (result.pos != 0) {
                    result.line += total_lines;
                    ++total_found;
                    ss << result.line << " ";
                    ss << result.pos << " ";
                    ss << result.found << std::endl;
                }
                else {
                    total_lines += result.line;
                }
            }
        }
        std::cout << total_found << std::endl;
        std::cout << ss.str();
    }

    void split_to_blocks(char* file_begin, uint64_t file_size, std::vector<Block>& blocks) {
        uint64_t avg_block_size = file_size / MAX_THREADS;
        char* file_end = file_begin + file_size - 1;

        char* block_begin = file_begin;
        uint64_t block_size = avg_block_size;
        char* block_end = block_begin + block_size - 1;

        while (true) {
            while (block_end < file_end && *block_end != '\n') {
                ++block_size;
                ++block_end;
            }

            Result empty(0, 0, "");
            Block new_block(block_begin, block_end, {empty});
            blocks.push_back(new_block);

            if (block_end == file_end) {
                break;
            }

            block_begin = block_end + 1;
            block_size = avg_block_size;
            block_end = block_begin + block_size - 1;
        }
    }

    int process_data(std::string file_name, std::string search_mask) {
        auto now1 = std::chrono::system_clock::now();
        auto time1 = std::chrono::system_clock::to_time_t(now1);
        //std::cout << ctime(&time1) << std::endl;
        mlen = search_mask.length();
        strcpy_s(mask, mlen + 1, search_mask.c_str());
        boost::iostreams::mapped_file file;
        file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);
        if (file.is_open()) {
            std::vector<Block> blocks;
            split_to_blocks((char*)file.const_data(), file.size(), blocks);
            std::vector <std::thread> threads;
            std::vector <Result> results;
            for (auto& block : blocks) {
                threads.push_back(std::thread(process_block, std::ref(block)));
            }
            for (auto& thread : threads) {
                thread.join();
            }
            merge_results(blocks);
            file.close();
        }
        else {
            std::cout << "could not map the file " << file_name << std::endl;
        }
        auto now2 = std::chrono::system_clock::now();
        auto time2 = std::chrono::system_clock::to_time_t(now2);
        //std::cout << ctime(&time2) << std::endl;
        std::chrono::duration<double> elapsed_seconds = now2 - now1;
        std::cout << "multi-threads: " << elapsed_seconds.count() << "s" << std::endl;
        return 0;
    }
}

int main(int argc, char* argv[]) {
    std::string file_name = "./resources/test.bin";
    std::string search_mask = "??fuck??";
    if (argc >= 3) {
        file_name = argv[1];
        search_mask = argv[2];
    }
    mtfind::process_data(file_name, search_mask);
}
