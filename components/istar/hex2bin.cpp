#include "hex2bin.h"

namespace hextools {

int8_t nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0; // Not a valid hexadecimal character
}


std::string hex2bin(const std::string& hexString)
{
    std::string byteArray;
    bool oddLength = hexString.length() & 1;

    int8_t currentByte = 0;

    for (int8_t charIndex = 0; charIndex < hexString.length(); charIndex++)
    {
        bool oddCharIndex = charIndex & 1;

        // If the length is odd
        if (oddLength == oddCharIndex)
        {
            // odd characters go in high nibble
            currentByte = nibble(hexString[charIndex]) << 4;
        }
        else
        {
            // Even characters go into low nibble
            currentByte |= nibble(hexString[charIndex]);
            byteArray += currentByte;
            currentByte = 0;
        }
    }

    return byteArray;
}

std::string hex2bin(const char* hexString)
{
    return hex2bin(std::string(hexString));
}

char anibble(uint8_t b)
{
    b &= 0x0F;
    return (b < 10) ? b + '0' : b - 10 + 'a';
}

std::string bin2hex(const std::string& bytes, char separator = 0)
{
    std::string res;
    size_t arraySize = bytes.length();

    for (int i = 0; i < arraySize; i++)
    {
        res += anibble(bytes[i] >> 4);
        res += anibble(bytes[i]);
        if (separator)
            res += separator;
    }

    return res;
}

std::string bin2hex(const uint8_t* bytes, size_t lenght)
{
    return bin2hex(std::string((const char *)bytes, lenght));
}

}