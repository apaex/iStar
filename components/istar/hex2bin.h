#pragma once
#include <string>
#include <stdint.h>

namespace hextools {

std::string hex2bin(const std::string& hexString);
std::string hex2bin(const char* hexString);
std::string bin2hex(const std::string& bytes, char separator);
std::string bin2hex(const uint8_t* bytes, size_t lenght);

}