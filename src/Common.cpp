#include "Common.hpp"

size_t ft_hash(const unsigned char *buf, size_t len) {
    size_t val = 0;

    for (size_t i = 0; i < len; i++)
        val += buf[i];
    return val;
}