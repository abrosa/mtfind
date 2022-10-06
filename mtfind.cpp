/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>

static const size_t THREADS = 24;
static const size_t BLOCK = 65536;

// Ulysses.txt ~ 23,6 blocks

void d_count(const int n, char* data, size_t length, int results[]) {
    int counter = 0;
    int position = 0;
    while (data && position < length) {
        if (*data == 'd') {
            ++counter;
        }
        ++data;
        ++position;
    }
    results[n] = counter;
}

int main() {
    std::vector<std::thread> threads;

    int results[THREADS] = {0};

    boost::iostreams::mapped_file file;

    std::string file_name = "Ulysses.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();

    size_t length = BLOCK;

    if (file.is_open()) {
        char* data = (char*) file.const_data();

        for (int i = 0; i < THREADS; i++) {
            if (i == THREADS - 1) {
                length = file_size - BLOCK * (THREADS - 1);
            }
            threads.push_back(std::thread(d_count, i, data + i * BLOCK, length, results));
        }

        for (auto& th : threads) {
            th.join();
        }

        for (int i = 0; i < THREADS; ++i) {
            std::cout << "Thread: " << i << ", Result: " << results[i] << std::endl;
        }

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
