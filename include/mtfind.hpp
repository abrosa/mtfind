/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <vector>
#include <string>

static const uint64_t C_IN_U64 = 8;

static const uint64_t MAX_THREADS = 16;

static const int MAX_MASK_LENGTH = 104;

static uint8_t wildcard[MAX_MASK_LENGTH] = { 0 };

static uint8_t masktext[MAX_MASK_LENGTH] = { 0 };

static size_t mlen;

static uint64_t masklen;

namespace mtfind {
    class Result {
    public:
        Result(uint64_t line, uint64_t pos, std::string found) {
            this->line = line;
            this->pos = pos;
            this->found = found;
        }
        uint64_t line;
        uint64_t pos;
        std::string found;
    };

    class Block {
    public:
        Block(char* begin, char* end, std::vector <Result> results) {
            this->begin = begin;
            this->end = end;
            this->results = results;
        }
        char* begin;
        char* end;
        std::vector <Result> results;
    };

    void process_block(Block& block);

    void merge_results(std::vector <Block>& blocks);

    void split_to_blocks(char* file_begin, uint64_t file_size, std::vector<Block>& blocks);

}

int main(int argc, char* argv[]);
