/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <../include/mtfind.hpp>
//#include <../include/compare_mask.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>

static const uint64_t MAX_THREADS = 16;
// static const uint64_t MAX_MASK_LENGTH = 100;
//static char mask[MAX_MASK_LENGTH];
static uint8_t wildcard[MAX_MASK_LENGTH] = { 0 };
static uint8_t masktext[MAX_MASK_LENGTH] = { 0 };
static uint64_t mlen;
static uint64_t masklen;

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

namespace mtfind {
    void print_it(uint64_t l) {
        for (int i = 0; i < CHARS_IN_X64; ++i) {
            int shift = 8 * i;
            int num = (l >> shift) & 255;
            if (num >= 32 && num <= 127) {
                std::cout << (char)num << "";
            }
            else {
                std::cout << num << "";
            }
        }
        std::cout << std::endl;
    }

    void process_block(Block & block) {
        std::vector <Result> results;
        uint64_t line = 0;
        char* i = block.begin;
        char* j;
        uint64_t k = 0;
        for (char* c = block.begin; c && c <= block.end; ++c) {
            if (*c != '\n' && c != block.end) {
                continue;
            }

            for (j = i; j <= c - 8 * masklen + 1; ++j) {
                uint64_t* lin = (uint64_t*)j;
                uint64_t* wld = (uint64_t*)wildcard;
                uint64_t* msk = (uint64_t*)masktext;
                uint64_t mu = 0;
                for (mu = 0; mu < masklen; ++mu) {
                    if ((*lin++ & *wld++) != *msk++) break;
                }
                if (mu == masklen) {
                    std::string found(j, mlen);
                    Result current(line, j - i + 1, found);
                    results.push_back(current);
                    j += mlen - 1;
                }
            }

            for (uint64_t nu = 0; j && j <= c - mlen + 1; ++j) {
                char* ll = j;
                char* ww = (char*)wildcard;
                char* mm = (char*)masktext;
                while (ll && (*ll & *ww) != *mm && ll <= c - mlen + 1) {
                    ++ll;
                }
                j = ll;
                for (nu = 0; nu < mlen; ++nu) {
                    if (mm != 0 && (*ll & *ww) != *mm) break;
                    ++ll;
                    ++ww;
                    ++mm;
                }
                if (nu == mlen) {
                    std::string found(j, mlen);
                    Result current(line, j - i + 1, found);
                    results.push_back(current);
                    j += mlen - 1;
                }
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
        uint64_t leftover = file_size - avg_block_size;

        while (true) {
            while (block_end < file_end && *block_end != '\n' && leftover != 0) {
                ++block_size;
                ++block_end;
                --leftover;
            }

            Result empty(0, 0, "");
            Block new_block(block_begin, block_end, {empty});
            blocks.push_back(new_block);

            if (leftover == 0) {
                break;
            }

            block_begin = block_end + 1;
            block_size = std::min(avg_block_size, leftover);
            block_end = block_begin + block_size - 1;
            leftover -= block_size;
        }
    }

    int process_data(std::string file_name, std::string search_mask) {
        // get file size
        // uint64_t file_size = filesize(file_name.c_str());
        // std::cout << file_size << std::endl;
        // return 0;
        auto now1 = std::chrono::system_clock::now();
        auto time1 = std::chrono::system_clock::to_time_t(now1);
        //std::cout << ctime(&time1) << std::endl;
        mlen = search_mask.length();
        for (int i = 0; i < mlen; ++i) {
            if (search_mask[i] == '?') {
                wildcard[i] = 0;
                masktext[i] = 0;
            }
            else {
                wildcard[i] = 255;
                masktext[i] = search_mask[i];
            }
        }
        masklen = mlen / 8;
        if (mlen % 8 != 0) {
            ++masklen;
        }

        //strcpy_s(mask, mlen + 1, search_mask.c_str());
        boost::iostreams::mapped_file_params params;
        params.path = file_name;
        params.flags = boost::iostreams::mapped_file::mapmode::readwrite;
        //params.length = file_size;
        boost::iostreams::mapped_file file;
        file.open(params);
        if (file.is_open()) {
            std::vector<Block> blocks;
            //char* data = (char*);
            //std::cout << "(" << file.end()-file.begin() << ")" << std::endl;
            //std::cout << "<" << data[file_size-1] << ">" << std::endl;
            split_to_blocks(file.data(), file.size(), blocks);
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
    std::string file_name = "./resources/hugetext.bin";
    std::string search_mask = "n?gger";
    if (argc >= 3) {
        file_name = argv[1];
        search_mask = argv[2];
    }
    mtfind::process_data(file_name, search_mask);
}
