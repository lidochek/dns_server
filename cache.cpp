#include "cache.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace cache {

std::map<std::vector<std::string>, Record> records;
static std::mutex cache_mutex;

std::vector<unsigned char> MakeAnswer(const std::vector<std::string>& domain, const std::string& ip) {
    parser::Header header{};
    header.qr = header.recursion_desired = header.recursion_available = true;
    header.question_count = header.answer_count = 1;
    parser::Question question{domain, parser::QueryType::A, parser::QueryClass::IN};
    parser::Resource answer{};
    answer.r_type = parser::QueryType::A;
    answer.r_class = parser::QueryClass::IN;
    answer.ttl = INFINITE_TTL;
    answer.r_length = 4;
    for (auto &part : domain) {
        answer.name.push_back(part.size());
        answer.name += part;
    }
    in_addr address{};
    if (inet_pton(AF_INET, ip.c_str(), &address) < 0) {
        perror("inet_pton config");
        throw std::runtime_error("Cannot convert config IP");
    }
    answer.r_data.assign((const char*)&address, 4);
    std::vector<unsigned char> response;
    parser::PackHeader(response, header);
    parser::PackQuestion(response, question);
    parser::PackResource(response, answer);
    return response;
}

void LoadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        perror("open config");
        throw std::runtime_error("Cannot open config: " + path);
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        std::vector<std::string> fields;
        std::string field;
        while (std::getline(row, field, '\t')) {
            fields.push_back(field);
        }
        if (!line.empty() && line.back() == '\t') {
            fields.push_back("");
        }
        if (fields.size() != 2) {
            continue;
        }
        std::istringstream parts(fields[0]);
        std::vector<std::string> domain;
        while (std::getline(parts, field, '.')) {
            domain.push_back(field);
        }
        records[domain] = {MakeAnswer(domain, fields[1]), std::chrono::steady_clock::time_point::max()};
    }
}

std::vector<unsigned char> Get(const parser::Question& question) {
    if (question.q_type != parser::QueryType::A || question.q_class != parser::QueryClass::IN) {
        return {};
    }
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto found = records.find(question.domain);
    if (found == records.end()) {
        return {};
    }
    if (std::chrono::steady_clock::now() >= found->second.expires_at) {
        records.erase(found);
        return {};
    }
    return found->second.response;
}

void Put(const parser::Question& question, const std::vector<unsigned char>& response) {
    if (question.q_type != parser::QueryType::A || question.q_class != parser::QueryClass::IN) {
        return;
    }
    try {
        auto header = parser::ParseHeader(response.data());
        if (!header.qr || header.response_code || header.trun_cation || response.size() > 512 || header.question_count != 1) {
            return;
        }
        size_t offset = parser::QUESTION_OFFSET;
        auto returned = parser::ParseQuestion(response.data() + offset, 1);
        if (returned.first[0].domain != question.domain || returned.first[0].q_type != question.q_type ||
            returned.first[0].q_class != question.q_class) {
            return;
        }
        offset += returned.second;
        uint64_t ttl = INFINITE_TTL;
        bool has_a = false;
        for (size_t i = 0; i < header.answer_count + header.name_service_count + header.additional_count; ++i) {
            auto resource = parser::ParseResource(response.data() + offset, 1);
            auto &answer = resource.first[0];
            has_a = has_a || (
                i < header.answer_count
                && answer.r_type == parser::QueryType::A
                && answer.r_class == parser::QueryClass::IN
                && answer.r_length == 4
            );
            if (answer.r_type != parser::QueryType::OPT) {
                ttl = std::min(ttl, answer.ttl > INFINITE_TTL ? 0 : answer.ttl);
            }
            offset += resource.second;
        }
        if (has_a && ttl && offset == response.size()) {
            std::lock_guard<std::mutex> lock(cache_mutex);
            records[question.domain] = {response, std::chrono::steady_clock::now() + std::chrono::seconds(ttl)};
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
    }
}

}
