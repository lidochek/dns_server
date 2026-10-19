#include "parser.hpp"
#include "helper.hpp"
#include <algorithm>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace parser {


Header ParseHeader(unsigned char* buffer) {
    std::cout << "\nParseHeader\n";

    Header result;
    result.id = helper::ParseNumber(buffer, 2);
    result.qr = buffer[2] & (1 << 7);
    std::cout << "KindQuery = " << ((buffer[2] & 0b0111'1000) >> 3) << '\n';
    result.opcode = static_cast<KindQuery>((buffer[2] & 0b0111'1000) >> 3);
    std::cout << "buffer[2] = " << std::bitset<8>{buffer[2]} << '\n';
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

std::pair<std::vector<Question>, size_t> ParseQuestion(unsigned char* buffer, uint64_t count) {
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
        std::cout << part << ".";
    }
    os << "\n";
    os << "Type: " << (int) quetion.q_type << "\n";
    os << "Class: " << (int) quetion.q_class << "\n";

    return os;
}

std::pair<std::vector<Resource>, size_t> ParseResource(unsigned char* buffer, uint64_t count){

    std::vector<Resource> result;
    size_t current_byte = 0;
    for (uint64_t i = 0; i < count; i++)
    {
        Resource resource;

        resource.name = "";
        while(buffer[current_byte]) {
            resource.name.push_back(buffer[current_byte]);
            ++current_byte;
        }
        ++current_byte;

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
