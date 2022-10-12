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
    stringstream result;
    ifstream input(file_name);
    int lines = 1;
    int matches = 0;

    smatch res;
    regex exp(mask_str);

    for (std::string line; getline(input, line); ++lines) {
        uint64_t pos = 0;
        while (regex_search(line.cbegin() + pos, line.cend(), res, exp)) {
            pos += res.position();
            result << lines << " " << pos + 1 << " " << res[0] << endl;
            pos += res.length();
            ++matches;
        }
    }
    cout << matches << endl;
    cout << result.str();
}

int main(int argc, char *argv[]) {
    auto now1 = chrono::system_clock::now();
    auto time1 = chrono::system_clock::to_time_t(now1);
    //cout << ctime(&time1) << endl;

    if (argc < 3) {
        file_name = "./resources/hugetext.bin";
        mask_str = "n.gger";
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
    cout << "regex: " << elapsed_seconds.count() << "s" << endl;
    return 0;
}
