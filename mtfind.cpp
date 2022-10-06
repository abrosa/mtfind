/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>

static const size_t BLOCK_SIZE = 65536;

// Ulysses.txt ~ 23,6 blocks

void d_count(const int n, char* bob, char* eob,
    std::vector<int> & results,
    std::vector<int> & lines,
    std::vector<int> & bytes) {
    int line = 1;
    int counter = 0;
    int by = 1;
    while (bob && bob < eob) {
        if (*bob == 'd') {
            ++counter;
        }
        if (*bob == '\n') {
            ++line;
        }
        ++bob;
        ++by;
    }
    bytes[n] = by;
    lines[n] = line;
    results[n] = counter;
}

int main() {
    std::vector<std::thread> threads;


    boost::iostreams::mapped_file file;

    std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";
    // std::string file_name = "dabadee.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();
    size_t max_blocks = 1 + file_size / BLOCK_SIZE;
    std::vector<int> results(max_blocks, 0);
    std::vector<int> lines(max_blocks, 0);
    std::vector<int> bytes(max_blocks, 0);
    size_t length = BLOCK_SIZE;

    if (file.is_open()) {
        char* data = (char*) file.const_data();
        char* bod = data;
        char* eod = data + file_size - 1;
        int i = 0;
        size_t position = 0;
        length = std::min(BLOCK_SIZE, file_size - position);
        char* bob = bod;
        char* eob = bob + length - 1;
        while (bob && eob) {
            while (eob && *eob != '\n') {
                ++eob;
            }
            if (!eob) {
                break;
            }
            threads.push_back(std::thread(d_count, i, bob, eob, std::ref(results), std::ref(lines), std::ref(bytes)));
            position += eob - bob + 1;
            if (position >= file_size) {
                break;
            }
            bob = eob + 1;
            if (!bob) {
                break;
            }
            length = std::min(BLOCK_SIZE, file_size - position);
            eob = bob + length - 1;
            if (!eob) {
                break;
            }
            ++i;
        }

        for (auto& th : threads) {
            th.join();
        }

        int total_bytes = 0;
        int total_d = 0;
        int total_lines = 0;
        for (int i = 0; i < max_blocks; ++i) {
            if (bytes[i] == 0) {
                break;
            }
            std::cout << "Thread: " << i;
            std::cout << ", Bytes: " << bytes[i];
            std::cout << ", Result: " << results[i];
            std::cout << ", Lines: " << lines[i] << std::endl;
            total_bytes += bytes[i];
            total_d += results[i];
            total_lines += lines[i];
        }
        std::cout << "Total bytes: " << total_bytes << std::endl;
        std::cout << "Total 'd': " << total_d << std::endl;
        std::cout << "Total lines: " << total_lines << std::endl;

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
