#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace helper
{
uint64_t ParseNumber(const unsigned char* buffer, size_t byte_count);
void PackNumber(std::vector<unsigned char>& buffer, uint64_t number, size_t byte_count);
bool ReadBytes(int socket, unsigned char* buffer, size_t size, bool allow_eof = false);
void SendBytes(int socket, const unsigned char* buffer, size_t size);
}//helper
