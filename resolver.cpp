#include "resolver.hpp"
#include "cache.hpp"
#include "helper.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace resolver {

std::vector<unsigned char> MakeResponse(parser::Header header, const std::vector<parser::Question>& questions) {
    header.qr = true;
    header.authoritative_answer = false;
    header.recursion_available = true;
    header.question_count = questions.size();
    header.answer_count = header.name_service_count = header.additional_count = 0;
    std::vector<unsigned char> result;
    parser::PackHeader(result, header);
    for (auto &question : questions) {
        parser::PackQuestion(result, question);
    }
    return result;
}

std::vector<unsigned char> ForwardQuery(parser::Header header, const parser::Question& question, bool tcp) {
    header.answer_count = header.name_service_count = header.additional_count = 0;
    std::vector<unsigned char> query;
    parser::PackHeader(query, header);
    parser::PackQuestion(query, question);
    int dns_socket = socket(PF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (dns_socket < 0) {
        perror("socket upstream");
        throw std::runtime_error("Dns socket error");
    }
    try {
        sockaddr_in addr{};
        addr.sin_family = PF_INET;
        addr.sin_port = htons(53);
        int status = inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
        if (status != 1) {
            if (status < 0) {
                perror("inet_pton upstream");
            }
            throw std::runtime_error("Dns address error");
        }
        if (connect(dns_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("connect upstream");
            throw std::runtime_error("Dns connect error");
        }
        std::vector<unsigned char> request, result(65535);
        if (tcp) {
            helper::PackNumber(request, query.size(), 2);
        }
        request.insert(request.end(), query.begin(), query.end());
        helper::SendBytes(dns_socket, request.data(), request.size());
        if (tcp) {
            unsigned char length[2];
            helper::ReadBytes(dns_socket, length, 2);
            result.resize(helper::ParseNumber(length, 2));
            helper::ReadBytes(dns_socket, result.data(), result.size());
        } else {
            ssize_t n = recv(dns_socket, result.data(), result.size(), 0);
            if (n < 0) {
                perror("recv upstream");
                throw std::runtime_error("Dns recv error");
            }
            result.resize(n);
        }
        if (close(dns_socket) < 0) {
            perror("close upstream");
        }
        return result;
    } catch (...) {
        if (close(dns_socket) < 0) {
            perror("close upstream");
        }
        throw;
    }
}

std::vector<unsigned char> Resolve(parser::Header header, const std::vector<parser::Question>& questions, bool tcp) {
    if (header.qr) {
        return {};
    }
    header.trun_cation = false;
    header.response_code = 0;
    if (questions.size() != 1 || header.opcode != parser::KindQuery::QUERY) {
        header.response_code = questions.size() != 1 ? 1 : 4;
        return MakeResponse(header, {});
    }
    auto cached = cache::Get(questions[0]);
    if (!cached.empty()) {
        cached[0] = header.id >> 8;
        cached[1] = header.id & 0xff;
        cached[2] = (cached[2] & 0xfe) | header.recursion_desired;
        return cached;
    }
    if (!header.recursion_desired) {
        header.response_code = 5;
        return MakeResponse(header, questions);
    }
    try {
        auto response = ForwardQuery(header, questions[0], tcp);
        cache::Put(questions[0], response);
        return response;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        header.response_code = 2;
        return MakeResponse(header, questions);
    }
}

}
