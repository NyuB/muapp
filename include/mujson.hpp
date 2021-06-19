/**
 * @file mujson.hpp
 * @author NyuB
 * @brief Helpers template to deal with Json formatter objects
 * @version 0.1
 * @date 2021-06-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef MUJSON_HEADER
#define MUJSON_HEADER
#include <cesanta/frozen.h>
#include <string>
#include <iostream>
#include <sstream>
namespace mujson {

template<typename T>
class JsonAdapter {
public:
    virtual ~JsonAdapter() {};
    virtual T fromJson(std::string json, bool * valid) = 0;
    virtual std::string toJson(T const&) = 0;
protected:
private:
};

}//namespace mujson
#endif //MUJSON_HEADER