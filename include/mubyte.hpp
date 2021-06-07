#ifndef MUBYTE_HEADER
#define MUBYTE_HEADER
#include <vector>
#include <string>
#include "mongoose.h"
typedef unsigned char byte;
std::vector<byte> cpy(const byte * src, size_t len);
inline std::string mgs2s(struct mg_str mgs){
    return std::string(mgs.ptr, mgs.len);
}
inline std::string mgs2s(struct mg_str * mgs){
    return std::string(mgs->ptr, mgs->len);
}
#endif