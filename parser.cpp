#include "parser.hpp"
#include <bitset>
#include <iostream>
#include <string>

namespace parser {


Header ParseHeader(unsigned char* buffer) {
    std::cout << "\nParseHeader\n";

    Header result;
    result.id = (buffer[0] << 8) | buffer[1];
    result.qr = buffer[2] & (1 << 7);
    std::cout << "KindQuery = " << ((buffer[2] & 0b0111'1000) >> 3) << '\n';
    result.opcode = static_cast<KindQuery>((buffer[2] & 0b0111'1000) >> 3);
    std::cout << "buffer[2] = " << std::bitset<8>{buffer[2]} << '\n';
    result.authoritative_answer = buffer[2] & 0b100;
    result.trun_cation = buffer[2] & 0b10;
    result.recursion_desired = buffer[2] & 0b1;
    result.recursion_available = buffer[3] & 0b1000'0000;
    result.response_code = buffer[3] & 0b1111;
    result.question_count =  (buffer[4] << 8) | buffer[5];
    result.answer_count =  (buffer[6] << 8) | buffer[7];
    result.name_service_count =  (buffer[8] << 8) | buffer[9];
    result.additional_count =  (buffer[10] << 8) | buffer[11];

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

Question ParseQuestion(unsigned char* buffer) {
    Question result;
    size_t length_block = 0;
    while(uint64_t length = buffer[length_block]) {
        std::string part;
        for (uint64_t i = 1; i <= length; i++) {
            part.push_back(buffer[length_block + i]);
        }
        length_block += length + 1;
        result.domain.emplace_back(std::move(part));
    }
    ++length_block;
    result.q_type = static_cast<QueryType>((buffer[length_block] << 8) | buffer[length_block+1]);
    length_block += 2;
    result.q_class = static_cast<QueryType>((buffer[length_block] << 8) | buffer[length_block+1]);
    return result;
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
}