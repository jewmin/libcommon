#include "test_log.h"

std::string ToHex(uint8_t c)
{
    char hex[3];

    const int val = c;

    sprintf(hex, "%02X", val);

    return hex;
}

std::string DumpData(const uint8_t * const data, size_t data_length, size_t line_length)
{
    const size_t bytes_per_line = line_length != 0 ? (line_length - 1) / 3 : 0;

    std::string result;

    std::string hex_display;
    std::string display;

    size_t i = 0;

    while (i < data_length)
    {
        const uint8_t c = data[i++];

        hex_display += ToHex(c) + " ";

        if (isprint(c))
        {
            display += (char)c;
        }
        else
        {
            display += '.';
        }

        if ((bytes_per_line && (i % bytes_per_line == 0 && i != 0)) || i == data_length)
        {
            result += hex_display + " - " + display + "\n";

            hex_display = "";
            display = "";
        }
    }

    return result;
}