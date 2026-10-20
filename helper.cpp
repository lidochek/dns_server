
#include "helper.hpp"
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <sys/socket.h>


namespace helper
{
uint64_t ParseNumber(const unsigned char* buffer, size_t byte_count) {
    uint64_t result = 0;
    for (size_t i = 0; i < byte_count; i++)
    {
        result = (result << 8ull) | (uint64_t) buffer[i];
    }
    return result;
}

void PackNumber(std::vector<unsigned char>& buffer, uint64_t number, size_t byte_count) {
    for (size_t i = byte_count; i > 0; --i)
    {
        buffer.push_back((number >> ((i - 1) * 8)) & 0xff);
    }
}
bool ReadBytes(int socket, unsigned char* buffer, size_t size, bool allow_eof) {
    ssize_t n = recv(socket, buffer, size, MSG_WAITALL);
    if (n < 0) {
        perror("recv");
        throw std::runtime_error("Socket recv error");
    }
    if (n == 0 && allow_eof) {
        return false;
    }
    if (n != (ssize_t)size) {
        throw std::runtime_error("Incomplete TCP read");
    }
    return true;
}

void SendBytes(int socket, const unsigned char* buffer, size_t size) {
    size_t offset = 0;
    while (offset < size) {
        ssize_t n = send(socket, buffer + offset, size - offset, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("send");
        }
        if (n <= 0) {
            throw std::runtime_error("Socket send error");
        }
        offset += n;
    }
}

}//helper
