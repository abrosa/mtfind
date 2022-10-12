#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <ctime>
#include <string>
#include <algorithm>

using namespace std;

static const uint64_t MAX_FILE_SIZE = 1024 * 1024 * 1024;

string file_name;
size_t file_size;
string mask_str;
int mask_len;

void process_data() {
    std::stringstream result;

    ifstream input(file_name);
    int lines = 1;
    int matches = 0;

    char* buffer(new char[MAX_FILE_SIZE]);
    input.read(buffer, file_size);

    char* begin = buffer;
    char* end = buffer + file_size - 1; 
    char* i = begin;
    char* j;
    uint64_t k;
    for (; begin < end; ++begin) {
        if (*begin != '\n')
            continue;
        for (j = i; j < begin - 1; ++j) {
            for (k = 0; k < mask_len; ++k) {
                if (!(j + k) || *(j + k) == '\n') break;
                if (!mask_str[k] || mask_str[k] == '\0') break;
                if (mask_str[k] != '?' && mask_str[k] != *(j + k)) break;
            }
            if (k != mask_len)
                continue;
            string found(j, mask_len);
            result << lines << " " << j - i + 1 << " " << found << endl;
            ++matches;
        }
        ++lines;
        i = begin + 1;
        if (i > end) {
            break;
        }
    }
    cout << matches << endl;
    cout << result.str();
}

int main(int argc, char *argv[]) {
    auto now1 = chrono::system_clock::now();
    auto time1 = chrono::system_clock::to_time_t(now1);
    //cout << ctime(&time1) << endl;
    //cout << argc << endl;
    if (argc < 3) {
        file_name = "./resources/hugetext.bin";
        mask_str = "n?gger";
    } else {
        file_name = argv[1];
        mask_str = argv[2];
    }

    ifstream in(file_name, ifstream::ate | ifstream::binary);
    file_size = in.tellg();
    mask_len = mask_str.length();

    process_data();
    
    auto now2 = chrono::system_clock::now();
    auto time2 = chrono::system_clock::to_time_t(now2);
    //cout << ctime(&time2) << endl;

    chrono::duration<double> elapsed_seconds = now2 - now1;
    cout << "no regex: " << elapsed_seconds.count() << "s" << endl;
    return 0;
}
