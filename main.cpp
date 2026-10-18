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
            auto parsed = parser::ParseHeader(buf);
            auto quetion = parser::ParseQuestion(buf + parser::QUESTION_OFFSET);
            std::cout << parsed;
            std::cout << quetion;
            
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
