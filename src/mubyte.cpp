#include "mubyte.hpp"
std::vector<byte> copy(byte * src, size_t len){
    std::vector<byte> res;
    res.reserve(len);
    res.assign(src, src+len);
    return res;
}