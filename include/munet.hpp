#ifndef MUNET_HEADER
#define MUNET_HEADER
#include <vector>
#include <memory>
#include <mongoose.h>
#include "mubyte.hpp"
namespace munet {
    
//Abstract Proxy Classes
template<typename T>
class ParsingStatus {
public:
    std::unique_ptr<T> get();
private:
    int code;
};

template<typename T, typename B>
class NetParser {
public:
    NetParser():input(NULL){};
    NetParser(B * i):input(i){};
    virtual ~NetParser() {};
    virtual size_t parse() {
        if(input!=NULL)return parse(input);
        return 0;
    };
protected:
    virtual size_t parse(B * src) = 0;
    B * input;
    ParsingStatus<T> status;
    std::vector<std::unique_ptr<T>> queue;
private:
};

template<typename T, typename B>
class NetSerializer {
public:
    NetSerializer():output(NULL){};
    NetSerializer(B * o):output(o){};
    virtual ~NetSerializer() {};
    virtual size_t serialize(T const& data) {
        if(output == NULL)return 0;
        return serialize(data, output);
    }
protected:
    virtual size_t serialize(T const&, B *) = 0;
    B * output;
private:
};

template<typename I, typename O>
class NetConverter {
public:
    virtual O convert(I const&) = 0;
private:
};


template<typename I, typename CLT, typename O, typename SRV>
class NetBridge {
public:
    NetBridge(NetParser<I *, CLT> in, NetSerializer<O *, SRV> out) : parser(in), serializer(out){};
    ~NetBridge(){};
protected:
    NetParser<I *, CLT> parser;
    NetSerializer<O *, SRV> serializer;
};

template<typename I, typename CLT, typename SRV>
class NetForward {
public:
    virtual SRV forward(I const&, CLT const&) = 0;
protected:
private:
};

//Mongoose Implementation
template<typename I>
class MgParser : public  NetParser<I, mg_connection> {
public:
    MgParser():NetParser<I, mg_connection>(){};
    MgParser(mg_connection * i):NetParser<I, mg_connection>(i){};
protected:
    virtual size_t parse(mg_connection * src) = 0;
private:
};

class MgHttpParser : public MgParser<mg_http_message> {
public:
    MgHttpParser():MgParser<mg_http_message>(){};
    MgHttpParser(mg_connection * i):MgParser<mg_http_message>(i){};
protected:
    virtual size_t parse(mg_connection * src) = 0;
};

template<typename O>
class MgSerializer : public NetSerializer<O, mg_connection> {
public:
    MgSerializer():NetSerializer<O, mg_connection>(){};
    MgSerializer(mg_connection * o):NetSerializer<O, mg_connection>(o){};
protected:
    virtual size_t serialize(O const&, mg_connection *) = 0;
};

class MgHttpSerializer : public MgSerializer<mg_http_message> {
public:
protected:
    virtual size_t serialize(mg_http_message const&, mg_connection *);
private:
};


class DNS {
public:
    virtual std::string resolve(std::string name) = 0;
};
void minimalHandler(mg_connection * c, int ev, void * ev_data, void * fn_data);
using DNSSharedPtr = std::shared_ptr<DNS>;
class MgHttpForward : public NetForward<mg_http_message, mg_connection *,  mg_connection *> {
public:
    virtual mg_connection * forward(mg_http_message const& data, mg_connection * const& clt);
protected:
private:
    mg_connection * forward_c = NULL;
    DNSSharedPtr dns;
};
}//namespace munet
#endif //MUNET_HEADER