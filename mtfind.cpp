/* Copyright [2022] <Alexander Abrosov> (aabrosov@gmail.com) */

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

namespace bio = boost::iostreams;

std::ifstream::pos_type filesize(const char *filename) {
  std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}

static const uint64_t MAX_THREADS = 16;
static const uint64_t MAX_MASK_LEN = 100;

uint64_t mask_len;
char *mask_str;

class Block {
 public:
  Block(uint64_t xblock, char *begin, char *end) {
    this->xblock = xblock;
    this->begin = begin;
    this->end = end;
  }
  uint64_t xblock;  // block num in 32 bits and line num in 32 bits
  char *begin;      // pointer to Begin
  char *end;        // pointer to End
};

class Result {
 public:
  Result(uint64_t xblock, uint64_t position, std::string found) {
    this->xblock = xblock;
    this->position = position;
    this->found = found;
  }
  uint64_t xblock;
  uint64_t position;
  std::string found;
};

void process_block(char *begin, char *end, uint64_t &xblock,
                   std::vector<Result> &results) {
  Result empty(xblock, 0, "");
  results.push_back(empty);
  char *i = begin;
  char *j;
  char *k;
  for (; begin <= end; ++begin) {
    if (*begin != '\n')
      continue;
    for (j = i; j <= begin - mask_len; ++j) {
      for (k = mask_str;
           k < mask_len + mask_str && (*k == '?' || *k == *(j - mask_str + k));
           ++k) {
      }
      if (k != mask_str + mask_len)
        continue;
      std::string found(j, mask_len);
      Result current(xblock, j - i + 1, found);
      results.push_back(current);
    }
    xblock++;
    i = begin + 1;
  }
}

void merge_results(std::vector<uint64_t> xblocks,
                   std::vector<Result> results) {
  uint64_t total_lines = 1;
  for (auto &xblock : xblocks) {
    for (auto &result : results) {
      if (result.xblock >> 32 == xblock >> 32) {
        result.xblock += total_lines;
      }
    }
    total_lines += xblock & 0xffffffff;
  }
  uint64_t total_found = 0;
  for (auto &result : results) {
    if (result.position != 0) {
      ++total_found;
    }
  }
  std::cout << total_found << std::endl;
  for (auto &result : results) {
    if (result.position != 0) {
      std::cout << (result.xblock & 0xffffffff) << " ";
      std::cout << result.position << " ";
      std::cout << result.found << std::endl;
    }
  }
}

std::vector<Block> split_to_blocks(char *data, uint64_t leftover) {
  std::vector<Block> blocks;
  Block block(0, data, data + leftover - 1);
  uint64_t block_size = leftover / MAX_THREADS;
  while (block.begin && block.end) {
    block.xblock += 0x100000000;
    if ((block.xblock >> 32) > MAX_THREADS) {
      break;
    }
    block.end = block.begin + std::min(block_size, leftover) - 1;
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
  return blocks;
}

int main(int argc, char *argv[]) {
  auto start_time = std::chrono::system_clock::now();
  // std::cout << "process started at " << start_time << std::endl;

  std::string file_name;
  std::string search_mask;
  if (argc < 3) {
    file_name = "./resources/test.bin";
    search_mask = "n?gger";
    // std::cout << "Usage info: mtfind.exe file.txt \"m?sk\"" << std::endl;
    // return -1;
  } else {
    file_name = argv[1];
    search_mask = argv[2];
  }

  mask_len = search_mask.length();
  mask_str = new char[MAX_MASK_LEN + 1];
  strcpy_s(mask_str, mask_len + 1, search_mask.c_str());
  size_t file_size = filesize(file_name.c_str());
  bio::mapped_file file;
  bio::mapped_file_params params;
  params.path = file_name;
  params.flags = bio::mapped_file::mapmode::readwrite;
  params.length = file_size + 1;
  file.open(params);
  if (file.is_open()) {
    char *data = (char *)(file.const_data());
    data[file_size] = '\n';
    std::vector<Block> blocks = split_to_blocks(data, file_size + 1);
    std::vector<std::thread> threads;
    std::vector<Result> results;
    std::vector<uint64_t> xblocks;
    for (auto &block : blocks) {
      uint64_t xblock = block.xblock;
      threads.push_back(std::thread(process_block, block.begin, block.end,
                                    std::ref(xblock), std::ref(results)));
      xblocks.push_back(xblock);
    }
    for (auto &thread : threads) {
      thread.join();
    }
    merge_results(xblocks, results);
    file.close();
  } else {
    std::cout << "could not map the file " << file_name << std::endl;
  }

  delete[] mask_str;

  auto end_time = std::chrono::system_clock::now();
  // std::cout << "process finished at " << end_time << std::endl;

  std::chrono::duration<double> elapsed_seconds = end_time - start_time;
  std::cout << "elapsed seconds: " << elapsed_seconds.count() << "s"
            << std::endl;
}
