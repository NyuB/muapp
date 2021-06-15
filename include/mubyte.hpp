#ifndef MUBYTE_HEADER
#define MUBYTE_HEADER
#include <vector>
#include <string>
#include <cesanta/mongoose.h>
typedef unsigned char byte;
/**
 * @brief Copy len bytes from src into the returned vector
 * 
 * @param src Byte buffer of at least len bytes
 * @param len Number of bytes to copy
 * @return std::vector<byte> 
 */
std::vector<byte> cpy(const byte * src, size_t len);

inline std::string mgs2s(struct mg_str mgs){
    return std::string(mgs.ptr, mgs.len);
}
inline std::string mgs2s(struct mg_str * mgs){
    return std::string(mgs->ptr, mgs->len);
}
#endif