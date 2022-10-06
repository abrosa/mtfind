/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <thread>
#include <vector>

static const size_t THREADS = 24;
static const size_t BLOCK = 65536;

// Ulysses.txt ~ 23,6 blocks

void d_count(const int n, char* data, char* eod, int results[], int lines[]) {
    int line = 1;
    int counter = 0;
    int position = 0;
    // char ch = (*data == '\n') ? '?' : *data;
    // std::cout << ch << "^" << n << "|" << line << "|" << counter << "|" << position << std::endl;
    while (data && data < eod) {
        if (*data == 'd') {
            ++counter;
        }
        if (*data == '\n') {
            ++line;
            ++data;
            ++position;
            // ch = (*data == '\n') ? '?' : *data;
            // std::cout << ch << "-" << n << "|" << line << "|" << counter << "|" << position << std::endl;
        }
        if (!data || data == eod) {
            break;
        }
        ++data;
        ++position;
        // ch = (*data == '\n') ? '?' : *data;
        // std::cout << ch << "_" << n << "|" << line << "|" << counter << "|" << position << std::endl;
    }

    lines[n] = line;
    results[n] = counter;
}

int main() {
    std::vector<std::thread> threads;

    int results[THREADS] = { 0 };
    int lines[THREADS] = { 0 };

    boost::iostreams::mapped_file file;

    std::string file_name = "Ulysses.txt";
    // std::string file_name = "example.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();

    size_t length = BLOCK;

    if (file.is_open()) {
        char* data = (char*) file.const_data();
        char* bod = data;
        char* eod = data + file_size - 1;
        int i = 0;
        size_t position = 0;
        length = std::min(BLOCK, file_size - position);
        char* bob = bod;
        char* eob = bob + length - 1;
        while (bob && eob) {
            // shift eob to next eol
            while (eob && *eob != '\n') {
                ++eob;
            }
            if (!eob) {
                break;
            }
            // threads.push_back(std::thread(d_count, i, bob, eob, results, lines));
            d_count(i, bob, eob, results, lines);
            position += eob - bob + 1;
            // std::cout << "processed up to position = " << position << std::endl;
            if (position >= file_size) {
                break;
            }
            bob = eob + 1;
            if (!bob) {
                break;
            }
            length = std::min(BLOCK, file_size - position);
            eob = bob + length - 1;
            if (!eob) {
                break;
            }
            ++i;
        }

        for (auto& th : threads) {
            th.join();
        }

        int total_d = 0;
        int total_lines = 0;
        for (int i = 0; i < THREADS; ++i) {
            std::cout << "Thread: " << i;
            std::cout << ", Result: " << results[i];
            std::cout << ", Lines: " << lines[i] << std::endl;
            total_d += results[i];
            total_lines += lines[i];
        }
        std::cout << "Total 'd': " << total_d << std::endl;
        std::cout << "Total lines: " << total_lines << std::endl;

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
