//https://cesanta.com/docs/
#ifndef MUAPP_HEADER
#define MUAPP_HEADER

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include "mongoose.h"
#include "frozen.h"

namespace muapp {

inline std::string mgs2s(struct mg_str mgs){
    return std::string(mgs.ptr, mgs.len);
}

typedef enum {
    GET,
    PUT,
    POST,
    DEL,
    NONE
}method;
method readMethod(mg_http_message * req);

class Response {
public:
    Response(struct mg_connection * c_);
    ~Response();
    Response * status(unsigned int status);
    Response * header(std::string const& key, std::string const& value);
    Response * write(std::string const& data);
    void send(unsigned int status, std::string const& data);
protected:
private:
    void reply();
    struct mg_connection * c;
    unsigned int statuscode = 200;
    std::map<std::string, std::string> headers;
    std::stringstream body;
};
using ResponseUPtr = std::unique_ptr<Response>;
using ResponseSharedPtr = std::shared_ptr<Response>;

class RequestCallback {
public:
    RequestCallback();
    virtual ~RequestCallback();
    virtual void call(mg_http_message * req, ResponseSharedPtr res) = 0;
    void operator()(mg_http_message * req, ResponseSharedPtr res);
};
using RequestCallbackSharedPtr = std::shared_ptr<RequestCallback>;
class FunctionPointerRequestCallback : public RequestCallback {
public:
    FunctionPointerRequestCallback(void cb_(mg_http_message *, ResponseSharedPtr res));
    ~FunctionPointerRequestCallback();
    virtual void call(mg_http_message * req, ResponseSharedPtr res);
private:
    void (*cb) (mg_http_message *, ResponseSharedPtr);
};

class API {
public:
    API();
    ~API();
    static void receive(struct mg_connection * c, int ev, void *ev_data, void *fn_data);
    API * useJson(void);
    API * useJson(bool b);
    void addRoute(method m, std::string uri, void cb(mg_http_message *, ResponseSharedPtr));
    void addRoute(method m, std::string uri, RequestCallbackSharedPtr cb);
    void addGlobRoute(method m, std::string globuri, void cb(mg_http_message *, ResponseSharedPtr));
    void addGlobRoute(method m, std::string globuri, RequestCallbackSharedPtr cb);
    void get(std::string uri, void cb(mg_http_message *, ResponseSharedPtr));
    void get(std::string uri, RequestCallbackSharedPtr cb);
    void put(std::string uri, void cb(mg_http_message *, ResponseSharedPtr));
    void put(std::string uri, RequestCallbackSharedPtr cb);
    void post(std::string uri, void cb(mg_http_message *, ResponseSharedPtr));
    void post(std::string uri, RequestCallbackSharedPtr cb);
    void del(std::string uri, void cb(mg_http_message *, ResponseSharedPtr)); 
    void del(std::string uri, RequestCallbackSharedPtr cb); 
protected:
    std::map<method,std::map<std::string, RequestCallbackSharedPtr>> routes;
    std::map<method,std::map<std::string, RequestCallbackSharedPtr>> globRoutes;
    bool json = false;
private:
};
class MuApp {
public:
    MuApp();
    ~MuApp();
    void listen(API * api, unsigned int port);
    void launch(void);
private:
    struct mg_mgr mgr;
};

} //namespace muapp
#endif //MUAPP_HEADER