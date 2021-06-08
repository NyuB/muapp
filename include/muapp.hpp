//https://cesanta.com/docs/
#ifndef MUAPP_HEADER
#define MUAPP_HEADER

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <cstring>
#include "mongoose.h"
#include "mujson.hpp"
#include "mubyte.hpp"
namespace muapp {

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
using ResponseUniquePtr = std::unique_ptr<Response>;

using RequestCallbackFunction = void (mg_http_message *, ResponseSharedPtr);
class RequestCallback {
public:
    RequestCallback();
    
    virtual ~RequestCallback();
    virtual void call(mg_http_message * req, ResponseSharedPtr res) = 0;
    void operator()(mg_http_message * req, ResponseSharedPtr res);
    
};
using RequestCallbackSharedPtr = std::shared_ptr<RequestCallback>;

template<typename J> 
class JsonRequestCallback : RequestCallback {
public:
    virtual void call(mg_http_message * req, ResponseSharedPtr res){
        J parsed = J().fromJson(mgs2s(req->body));
        if(parsed.isValid()){
            jcb(req, parsed, res);
        }
        else{
            res->send(400, "Invalid JSon Body");
        }
        
    }
protected:
    void jcb (mg_http_message *, J const&, ResponseSharedPtr) = 0;
};

class FunctionPointerRequestCallback : public RequestCallback {
public:
    FunctionPointerRequestCallback(RequestCallbackFunction cb);
    FunctionPointerRequestCallback(const RequestCallbackFunction&);
    ~FunctionPointerRequestCallback();
    virtual void call(mg_http_message * req, ResponseSharedPtr res);
private:
    RequestCallbackFunction * cb;
};

class API {
public:
    API();
    ~API();
    /*
        Core callback method to redirect mg_connection to
        Handle mg_http_message events and dispatch them to API's routes
        Ignore other events
        Can be used as is or delegated to a MuApp encapsulation to handle socket initialization/closure as well
        (see muapp::MuApp::listen, muapp::attach)
        fn_data should hold an API* object
    */
    static void receive(struct mg_connection * c, int ev, void *ev_data, void *fn_data);
    
    /*
        Flag to automate Content-Type: application/json header
    */
    API * useJson(void);
    API * useJson(bool b);

    /*
        Core routing/functions mapping
    */
    void addRoute(method m, std::string uri, RequestCallbackFunction cb);
    void addRoute(method m, std::string uri, RequestCallbackSharedPtr cb);
    
    /*
        Behaves the same but uses glob syntax to match uri
        Routes added with addRoute have precedence over glob routes when checking matches
    */
    void addgRoute(method m, std::string globuri, RequestCallbackFunction);
    void addgRoute(method m, std::string globuri, RequestCallbackSharedPtr cb);

    /*
        Aliases for addRoute/addgRoute for common http verbs
    */
    void get(std::string uri, RequestCallbackFunction cb);
    void get(std::string uri, RequestCallbackSharedPtr cb);
    void put(std::string uri, RequestCallbackFunction cb);
    void put(std::string uri, RequestCallbackSharedPtr cb);
    void post(std::string uri, RequestCallbackFunction cb);
    void post(std::string uri, RequestCallbackSharedPtr cb);
    void del(std::string uri, RequestCallbackFunction cb); 
    void del(std::string uri, RequestCallbackSharedPtr cb); 
    void gget(std::string uri, RequestCallbackFunction cb);
    void gget(std::string uri, RequestCallbackSharedPtr cb);
    void gput(std::string uri, RequestCallbackFunction cb);
    void gput(std::string uri, RequestCallbackSharedPtr cb);
    void gpost(std::string uri, RequestCallbackFunction cb);
    void gpost(std::string uri, RequestCallbackSharedPtr cb);
    void gdel(std::string uri, RequestCallbackFunction cb); 
    void gdel(std::string uri, RequestCallbackSharedPtr cb); 

    
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
    void listen(API * api, unsigned int port);//Listen using Api as handler function
    void listen(mg_event_handler_t fn, void * fn_data, unsigned int port);//Mongoose http listening
    void listenRaw(mg_event_handler_t fn, void * fn_data, unsigned int port);//Mongoose tcp listening
    void launch(void);
private:
    std::string listenUrl(unsigned int port);
    struct mg_mgr mgr;
};
void attach(API * api, MuApp * app, unsigned int port);

} //namespace muapp
#endif //MUAPP_HEADER