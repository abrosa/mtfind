/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <vector>
#include <string>

static const int CHARS_IN_X64 = 8;
// rounded up to 8th
static const int MAX_MASK_LENGTH = 104;

// '?' == 0x3F == 63
static const uint64_t qq = 63ull;
static const uint64_t q[CHARS_IN_X64] = {
    qq, (qq << 8), (qq << 16), (qq << 24), (qq << 32), (qq << 40), (qq << 48), (qq << 56)
};

// '\n' == 0x0A == 10
static const uint64_t nn = 10ull;
static const uint64_t n[CHARS_IN_X64] = {
    nn, (nn << 8), (nn << 16), (nn << 24), (nn << 32), (nn << 40), (nn << 48), (nn << 56)
};

// mask == 0xFF == 255
static const uint64_t ff = 255ull;
static const uint64_t f[CHARS_IN_X64] = {
    ff, (ff << 8), (ff << 16), (ff << 24), (ff << 32), (ff << 40), (ff << 48), (ff << 56)
};

inline const char* const BoolToString(bool b) {
    return b ? "true" : "false";
}

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
        bool operator > (const Result& res) const {
            return (line > res.line);
        }
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
