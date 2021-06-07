#ifndef MUJSON_HEADER
#define MUJSON_HEADER
#include "frozen.h"
#include <string>
#include <iostream>
#include <sstream>
namespace mujson {
template<typename T>
class JsonAdapter {
public:
    virtual ~JsonAdapter() {};
    virtual T fromJson(std::string json) = 0;
    virtual std::string toJson(T const&) = 0;
protected:
private:
};

}//namespace mujson
#endif //MUJSON_HEADER