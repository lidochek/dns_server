#include <cstddef>
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>

#include "parser.hpp"
#include "helper.hpp"
#include "resolver.hpp"
#include "cache.hpp"

static in_addr server_address{htonl(INADDR_LOOPBACK)};

void RunTcpServer() {
    int tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        perror("Tcp socket");
        throw std::runtime_error("Wrong Soket");
    }
    
    int reuse = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt TCP");
    }
    sockaddr_in addr{};
    addr.sin_family = PF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr = server_address;

    if (bind(tcp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind TCP");
        throw std::runtime_error("Bind error");
    }
    if (listen(tcp_socket,3) < 0) {
        perror("listen TCP");
        throw std::runtime_error("Listen error");
    }

    while (true) {
        int client_fd = accept(tcp_socket, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept TCP");
            continue;
        }
        try {
            unsigned char length[2];
            while (helper::ReadBytes(client_fd, length, 2, true)) {
                std::vector<unsigned char> buf(helper::ParseNumber(length, 2));
                helper::ReadBytes(client_fd, buf.data(), buf.size());
                parser::Header parsed{};
                std::vector<parser::Question> questions;
                try {
                    if (buf.size() < parser::QUESTION_OFFSET) {
                        throw std::runtime_error("Short DNS header");
                    }
                    parsed = parser::ParseHeader(buf.data());
                    questions = parser::ParseQuestion(buf.data() + parser::QUESTION_OFFSET, parsed.question_count).first;
                } catch (const std::exception& error) {
                    std::cerr << error.what() << '\n';
                    parsed.qr = false;
                    parsed.opcode = parser::KindQuery::QUERY;
                }
                auto response = resolver::Resolve(parsed, questions, true);
                if (response.empty()) {
                    continue;
                }
                std::vector<unsigned char> result;
                helper::PackNumber(result, response.size(), 2);
                result.insert(result.end(), response.begin(), response.end());
                helper::SendBytes(client_fd, result.data(), result.size());
            }
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
        }
        if (close(client_fd) < 0) {
            perror("close TCP client");
        }
    }
}
void RunUdpServer() {
    auto udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("Udp socket");
        throw std::runtime_error("Wrong Soket");
    }

    sockaddr_in addr{};
    addr.sin_family = PF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr = server_address;

    if (bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind UDP");
        throw std::runtime_error("Bind error");

    }

    while (true) {
        unsigned char buf[65535];
        sockaddr_in client{};
        socklen_t client_size = sizeof(client);
        while (
            int n = recvfrom(udp_socket, &buf, sizeof(buf), 0, (sockaddr*)&client, &client_size)
        ) {
            if (n < 0) {
                perror("recvfrom");
                throw std::runtime_error("Recvfrom error");
            }
            parser::Header parsed{};
            if (n < 2) {
                continue;
            }
            std::vector<unsigned char> response;
            try {
                if (n < (int)parser::QUESTION_OFFSET) {
                    throw std::runtime_error("Short DNS header");
                }
                parsed = parser::ParseHeader(buf);
                size_t offset = parser::QUESTION_OFFSET;
                auto quetion = parser::ParseQuestion(buf + offset, parsed.question_count);
                offset += quetion.second;

                auto answer = parser::ParseResource(buf + offset, parsed.answer_count);
                offset += answer.second;

                auto authority = parser::ParseResource(buf + offset, parsed.name_service_count);
                offset += authority.second;

                auto additional = parser::ParseResource(buf + offset, parsed.additional_count);
                offset += additional.second;

                response = resolver::Resolve(parsed, quetion.first, false);
            } catch (const std::exception& error) {
                std::cerr << error.what() << '\n';
                parsed.qr = false;
                parsed.opcode = parser::KindQuery::QUERY;
                response = resolver::Resolve(parsed, {}, false);
            }
            if (!response.empty() && sendto(udp_socket, response.data(), response.size(), 0,
                                           (sockaddr*)&client, client_size) < 0) {
                perror("sendto");
            }
        }

    }
}



int	main(int argc, char **argv)
{
    try {
        if (argc > 1) {
            int status = inet_pton(AF_INET, argv[1], &server_address);
            if (status != 1) {
                if (status < 0) {
                    perror("inet_pton listen");
                }
                throw std::runtime_error("Wrong listen address");
            }
        }
        cache::LoadConfig("records.tsv");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    std::thread tcp(RunTcpServer);
    std::thread udp(RunUdpServer);

    tcp.join();
    udp.join();

    return 0;
}
