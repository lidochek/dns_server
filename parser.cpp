#include "parser.hpp"
#include "helper.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

namespace parser {


Header ParseHeader(const unsigned char* buffer) {
    Header result{};
    result.id = helper::ParseNumber(buffer, 2);
    result.qr = buffer[2] & (1 << 7);
    result.opcode = static_cast<KindQuery>((buffer[2] & 0b0111'1000) >> 3);
    result.authoritative_answer = buffer[2] & 0b100;
    result.trun_cation = buffer[2] & 0b10;
    result.recursion_desired = buffer[2] & 0b1;
    result.recursion_available = buffer[3] & 0b1000'0000;
    result.response_code = buffer[3] & 0b1111;
    result.question_count =  helper::ParseNumber(buffer + 4, 2);
    result.answer_count =  helper::ParseNumber(buffer + 6, 2);
    result.name_service_count =  helper::ParseNumber(buffer + 8, 2);
    result.additional_count =  helper::ParseNumber(buffer + 10, 2);

    return result;
}

std::ostream& operator<< (std::ostream& os, const Header& header) {
    os << "Header.\n" << std::dec;
    os << "\n Id: " << header.id;
    os << "\n qr: " << header.qr;
    os << "\n opcode: " << (int) header.opcode;
    os << "\n authoritative_answer: " << header.authoritative_answer;
    os << "\n trun_cation: " << header.trun_cation;
    os << "\n recursion_desired: " << header.recursion_desired;
    os << "\n recursion_available: " << header.recursion_available;
    os << "\n response_code: " << header.response_code;
    os << "\n question_count: " << header.question_count;
    os << "\n answer_count: " << header.answer_count;
    os << "\n name_service_count: " << header.name_service_count;
    os << "\n additional_count: " << header.additional_count;
    os << "\n";
    return os;
}

std::pair<std::vector<Question>, size_t> ParseQuestion(const unsigned char* buffer, uint64_t count) {
    std::vector<Question> result;
    size_t current_byte = 0;
    for (uint64_t i = 0; i < count; i++)
    {
        Question question;
        while(uint64_t length = buffer[current_byte]) {
            std::string part;
            for (uint64_t i = 1; i <= length; i++) {
                part.push_back(buffer[++current_byte]);
            }
            ++current_byte;
            question.domain.emplace_back(std::move(part));
        }
        ++current_byte;
        question.q_type = static_cast<QueryType>(helper::ParseNumber(buffer + current_byte, 2));
        current_byte += 2;
        question.q_class = static_cast<QueryClass>(helper::ParseNumber(buffer + current_byte, 2));
        current_byte += 2;
        result.emplace_back(std::move(question));
    }

    return {result, current_byte};
}

std::ostream& operator<< (std::ostream& os, const Question& quetion) {
    os << "Question.\n" << std::dec;
    for (auto &part : quetion.domain)
    {
        os << part << ".";
    }
    os << "\n";
    os << "Type: " << (int) quetion.q_type << "\n";
    os << "Class: " << (int) quetion.q_class << "\n";

    return os;
}

std::pair<std::vector<Resource>, size_t> ParseResource(const unsigned char* buffer, uint64_t count) {
    std::vector<Resource> result;
    size_t current_byte = 0;
    for (uint64_t i = 0; i < count; i++)
    {
        Resource resource;
        resource.name = "";
        bool pointer = false;
        while(uint64_t length = buffer[current_byte]) {
            pointer = (length & 0xc0) == 0xc0;
            size_t count = pointer ? 2 : length + 1;
            for (size_t i = 0; i < count; ++i) {
                resource.name.push_back(buffer[current_byte++]);
            }
            if (pointer) {
                break;
            }
        }
        if (!pointer) {
            ++current_byte;
        }

        resource.r_type = static_cast<QueryType>(helper::ParseNumber(buffer + current_byte, 2));
        current_byte += 2;
        resource.r_class = static_cast<QueryClass>(helper::ParseNumber(buffer + current_byte, 2));
        current_byte += 2;
        resource.ttl = helper::ParseNumber(buffer + current_byte, 4);
        current_byte += 4;
        resource.r_length = helper::ParseNumber(buffer + current_byte, 2);
        current_byte += 2;
        resource.r_data = "";
        for (uint64_t i = 0; i < resource.r_length; ++i, ++current_byte)
        {
            resource.r_data.push_back(buffer[current_byte]);
        }
        result.emplace_back(std::move(resource));
    }

    return {result, current_byte};
}

void PackHeader(std::vector<unsigned char>& buffer, const Header& header) {
    helper::PackNumber(buffer, header.id, 2);
    buffer.push_back((header.qr << 7) | ((int)header.opcode << 3) |
                    (header.authoritative_answer << 2) | (header.trun_cation << 1) |
                    header.recursion_desired);
    buffer.push_back((header.recursion_available << 7) | header.response_code);
    helper::PackNumber(buffer, header.question_count, 2);
    helper::PackNumber(buffer, header.answer_count, 2);
    helper::PackNumber(buffer, header.name_service_count, 2);
    helper::PackNumber(buffer, header.additional_count, 2);
}

void PackQuestion(std::vector<unsigned char>& buffer, const Question& question) {
    for (auto &part : question.domain)
    {
        buffer.push_back(part.size());
        buffer.insert(buffer.end(), part.begin(), part.end());
    }
    buffer.push_back(0);
    helper::PackNumber(buffer, (int)question.q_type, 2);
    helper::PackNumber(buffer, (int)question.q_class, 2);
}

void PackResource(std::vector<unsigned char>& buffer, const Resource& resource) {
    buffer.insert(buffer.end(), resource.name.begin(), resource.name.end());
    buffer.push_back(0);
    helper::PackNumber(buffer, (int)resource.r_type, 2);
    helper::PackNumber(buffer, (int)resource.r_class, 2);
    helper::PackNumber(buffer, resource.ttl, 4);
    helper::PackNumber(buffer, resource.r_data.size(), 2);
    buffer.insert(buffer.end(), resource.r_data.begin(), resource.r_data.end());
}

std::ostream& operator<< (std::ostream& os, const Resource& rs) {
    os << "Resource.\n" << std::dec;

    os << "Name size: " << rs.name.size() << "\n";
    os << "Name: " << rs.name << "\n";

    os << "Type: " << (int) rs.r_type << "\n";
    os << "Class: " << (int) rs.r_class << "\n";

    os << "Ttl: " << rs.ttl << "\n";
    os << "R length: " << rs.r_length << "\n";
    os << "R string: " << rs.r_data << "\n";

    return os;
}

}
