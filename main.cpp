#include <cstddef>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>

#include "parser.hpp"


void RunTcpServer() {
    int tcp_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        perror("Tcp socket");
        throw std::runtime_error("Wrong Soket");
    }
    
    sockaddr_in addr{};
    addr.sin_family = PF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(tcp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
    }
    if (listen(tcp_socket,3) < 0) {
        perror("listen");
    }

    while (true) {
        int client_fd = accept(tcp_socket, nullptr, nullptr);
        char buf[1024];
        while (
            int n = read(client_fd, &buf, sizeof(buf))
        ) {
            for (int i = 0; i < n; i++)
            {
                std::cout << buf[i];
            }
        }
        std::cout<< std::endl << "==============TCP==============" << std::endl;
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
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(udp_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        throw std::runtime_error("Bind error");

    }

    while (true) {
        unsigned char buf[1024];
        std::cout << std::hex;
        while (
            int n = recvfrom(udp_socket, &buf, sizeof(buf), 0, nullptr, nullptr)
        ) {
            if (n < 0) {
                perror("recvfrom");
                throw std::runtime_error("Recvfrom error");
            }
            for (int i = 0; i < n; i++)
            {
                std::cout << (int) buf[i] << ' ';
            }
            std::cout << std::dec;
            auto parsed = parser::ParseHeader(buf);
            size_t offset = parser::QUESTION_OFFSET;
            std::cout << "question_offset = " << offset  << "\n";
            auto quetion = parser::ParseQuestion(buf + offset, parsed.question_count);
            offset += quetion.second;

            std::cout << "answer_offset = " << offset  << "\n";


            auto answer = parser::ParseResource(buf + offset, parsed.answer_count);
            offset += answer.second;

            std::cout << "authority_offset = " << offset  << "\n";


            auto authority = parser::ParseResource(buf + offset, parsed.name_service_count);
            offset += authority.second;

            std::cout << "additional_offset = " << offset  << "\n"; 


            auto additional = parser::ParseResource(buf + offset, parsed.additional_count);
            offset += additional.second;

            std::cout << parsed;
            std::cout << quetion.first[0];
            std::cout << "Answer size: " << answer.first.size() << "\n";
            if (!answer.first.empty()) {
                std::cout <<  answer.first[0];
            }
            std::cout << "Authority size: " << authority.first.size()  << "\n";
            if (!authority.first.empty()) {
                std::cout << authority.first[0];
            } 
            std::cout << "Additional size: " << additional.first.size()  << "\n";
            if (!additional.first.empty()) {
                std::cout << additional.first[0];
            } 
            // std::cout << "Additional size: " << additional.size()  << "\n" <<  (additional.empty() ? parser::Resource{}: additional[0]);
            
            std::cout<< std::endl << "===============UDP=============" << std::endl;
        }

    }
}



int	main(int argc, char **argv)
{
    std::cout << std::hex;
    std::thread tcp(RunTcpServer);
    std::thread udp(RunUdpServer);

    tcp.join();
    udp.join();

    return 0;
}
