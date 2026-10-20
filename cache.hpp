#pragma once
#include "parser.hpp"
#include <chrono>
#include <map>
#include <limits>

namespace cache {

constexpr uint64_t INFINITE_TTL = std::numeric_limits<int32_t>::max();

struct Record {
    std::vector<unsigned char> response;
    std::chrono::steady_clock::time_point expires_at;
};

// One shared map, defined in cache.cpp.
extern std::map<std::vector<std::string>, Record> records;

void LoadConfig(const std::string& path);
std::vector<unsigned char> MakeAnswer(const std::vector<std::string>& domain, const std::string& ip);
std::vector<unsigned char> Get(const parser::Question& question);
void Put(const parser::Question& question, const std::vector<unsigned char>& response);

}
