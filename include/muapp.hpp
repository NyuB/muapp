/**
 * @file muapp.hpp
 * @author NyuB
 * @brief Minimal "Express-ish" api to build a web server relying on mongoose
 * @version 0.1
 * @date 2021-06-19
 * 
 * @copyright Copyright (c) 2021
 * 
 * @see https://cesanta.com/docs/
 */
#ifndef MUAPP_HEADER
#define MUAPP_HEADER

#include <mutex>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <cstring>
#include <cesanta/mongoose.h>
#include "mujson.hpp"
#include "mubyte.hpp"
#define MUAPP_RSC_READ(Resource, Name, Fun) class Name : public muapp::RequestCallback {public:Name(std::shared_ptr<const Resource> r):resource(r){};~Name(){};virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res){Fun};private:std::shared_ptr<const Resource> resource;} 
#define MUAPP_RSC_READ_J(Resource, Name, JType, Fun) class Name : public muapp::JsonRequestCallback<JType> {public:Name(std::shared_ptr<const Resource> r):resource(r){};~Name(){};virtual void jcb(mg_http_message * req, JType const& obj, muapp::ResponseSharedPtr res){Fun};private:std::shared_ptr<const Resource> resource;}
#define MUAPP_RSC_MUT(Resource, Name, Fun) class Name : public muapp::RequestCallback {public:Name(std::shared_ptr<Resource> r):resource(r){};~Name(){};virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res){Fun};private:std::shared_ptr<Resource> resource;} 
#define MUAPP_RSC_MUT_J(Resource, Name, JType, Fun) class Name : public muapp::JsonRequestCallback<JType> {public:Name(std::shared_ptr<Resource> r):resource(r){};~Name(){};virtual void jcb(mg_http_message * req, JType const& obj, muapp::ResponseSharedPtr res){Fun};private:std::shared_ptr<Resource> resource;}
namespace muapp {

typedef enum {
    GET,
    PUT,
    POST,
    DEL,
    NONE
}method;
method readMethod(mg_http_message * req);

/**
 * @brief Http response handler, designed to be passed among callbacks and send correctly formatted response upon destruction
 * 
 * @note Content-Length/Encoding headers will be correctly set upon response sending
 * 
 * @note Methods returning Response * are designed to be chained, methods returning void are designed to be final
 */
class Response {
public:
    /**
     * @brief Construct a new Response object and links it to the client to serve
     * 
     * @param c_ The client connection to send response to
     */
    Response(struct mg_connection * c_);
    /**
     * @brief Destroy the Response object after sending it's content
     * 
     */
    ~Response();
    /**
     * @brief Set the status code of the response
     * 
     * @param status HTTP return code
     * @return Response* The called Response object
     */
    Response * status(unsigned int status);
    /**
     * @brief Set the corresponding header in response
     * @note Headers' name and value support only US ASCII encoding
     * @param name Case-insensitive ASCII header name 
     * @param value US ASCII header value
     * @return Response* The called Response object
     */
    Response * header(std::string const& name, std::string const& value);
    /**
     * @brief Append data to the response body
     * 
     * @param data The bytes to append to the response body 
     * @return Response* The called Response object
     */
    Response * write(std::string const& data);
    /**
     * @brief Acquires the given mutex before accessing i/o context to reply
     * 
     * @param m A mutex guarding the i/o context
     * @return Response* The called Response object
     */
    Response * synchronize(std::mutex * m);

    /**
     * @brief Shorthand for ->status(code)->write(data)
     * 
     * @param status HTTP return code
     * @param data Bytes to append to response body
     */
    void send(unsigned int status, std::string const& data);

    /**
     * @brief ->status(400)
     * 
     */
    void badRequest();

    /**
     * @brief ->status(200)
     * 
     */
    void ok();

    /**
     * @brief ->send(200, data)
     * 
     * @param data body data to append
     */
    void ok(std::string data);

    /**
     * @brief ->status(404)
     * 
     */
    void notFound();

    /**
     * @brief ->status(500)
     * 
     */
    void error();
protected:
private:
    std::mutex * synchro = NULL;
    struct mg_connection * c;
    unsigned int statuscode = 200;
    std::map<std::string, std::string> headers;
    std::stringstream body;
};
using ResponseSharedPtr = std::shared_ptr<Response>;
using ResponseUniquePtr = std::unique_ptr<Response>;

using RequestCallbackFunction = void (mg_http_message *, ResponseSharedPtr);
/**
 * @brief Abstract mg_http_message handler
 * 
 * @see @ref muapp::Response
 */
class RequestCallback {
public:
    RequestCallback();
    virtual ~RequestCallback();
    /**
     * @brief Abstract HTTP request handler
     * 
     * @param req HTTP request triggering this handler
     * @param res HTTP response, will be send when all related shared pointer have been released
     */
    virtual void call(mg_http_message * req, ResponseSharedPtr res) = 0;
    /**
     * @brief HTTP request handler
     * 
     * @param req HTTP request triggering this handler
     * @param res HTTP response, will be send when all related shared pointer have been released
     */
    void operator()(mg_http_message * req, ResponseSharedPtr res);
};
using RequestCallbackSharedPtr = std::shared_ptr<RequestCallback>;

/**
 * @brief Convenience RequestCallback Wrapper for JSon HTTP request
 * 
 * @tparam J A type accepting a J::fromJson(std::string) static method;
 */
template<class J> 
class JsonRequestCallback : public RequestCallback {
public:
    void call(mg_http_message * req, ResponseSharedPtr res){
        bool r;
        J parsed = J::fromJson(mgs2s(req->body), &r);
        if(r)jcb(req, parsed, res);
        else{
            res->send(400,"");
        } 
    }
    /**
     * @brief HTTP Request handler with preparsed JSon object
     * 
     * @param r The origin request from where obj was parsed
     * @param obj The parsed obj
     * @param res @ref muapp::RequestCallback
     */
    virtual void jcb (mg_http_message * r, J const& obj, ResponseSharedPtr res) = 0;
};

/**
 * @brief Specialization to map with a JSonAdapter instead of a direct static class method
 * 
 * @tparam J 
 */
template<class J>
class JsonRequestCallback<mujson::JsonAdapter<J>> : public RequestCallback {
public:
    JsonRequestCallback(mujson::JsonAdapter<J> adapter_):adapter(adapter_){};
    /**
     * @brief muapp::RequestCallback implementation, preparse http body as json
     * 
     * @param req 
     * @param res 
     */
    void call(mg_http_message * req, ResponseSharedPtr res){
        bool r;
        J parsed = adapter.fromJson(mgs2s(req->body), &r);
        if(r)jcb(req, parsed, res);
        else{
            res->send(400,"");
        } 
    }
    /**
     * @brief HTTP Request handler with preparsed JSon object
     * 
     * @param r The origin request from where obj was parsed
     * @param obj The parsed obj
     * @param res @ref muapp::RequestCallback
     */
    virtual void jcb (mg_http_message * r, J const& obj, ResponseSharedPtr res) = 0;
protected:
    mujson::JsonAdapter<J> adapter;
};

/**
 * @brief Wrapper around direct function pointers implementing RequestCallback
 * 
 */
class FunctionPointerRequestCallback : public RequestCallback {
public:
    FunctionPointerRequestCallback(RequestCallbackFunction cb);
    FunctionPointerRequestCallback(const RequestCallbackFunction&);
    ~FunctionPointerRequestCallback();
    virtual void call(mg_http_message * req, ResponseSharedPtr res);
private:
    RequestCallbackFunction * cb;
};

/**
 * @brief Routes/Handlers container mapping URIs to callback handler objects, triggered upon reception of http messages
 * 
 * @see https://cesanta.com/docs/
 * @see @ref muapp::RequestCallback
 * @see @ref muapp::MuApp
 */
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

    /**
     * @brief Core callback method to pass as mg_http_listen callback. Handle mg_http_message events and dispatch them to API's routes
     * 
     * @note If an API is registered to a MuApp handler, this function would be call internally and should not be used raw
     * 
     * @param c A client connection, or the related listening server for EV_CLOSE and EV_ERROR events
     * @param ev The event code of the Mongoose server framework
     * @param ev_data Event data related to the event code
     * @param fn_data Expects an API* object or NULL
     */
    static void receive(struct mg_connection * c, int ev, void *ev_data, void *fn_data);
    
    /**
     * @brief Set Json flag and send/expect "Content-Type: application/json" header and content
     * 
     * @return API* The called API object
     */
    API * useJson(void);
    /**
     * @brief Toggle Json flag to send/expect "Content-Type: application/json" header and content
     * 
     * @return API* The called API object
     */
    API * useJson(bool b);

    /**
     * @brief Core route/handler mapping method matching absolute URIs
     * @note Absolute matching takes priority over glob pattern
     * 
     * @see @ref muapp::API::addgRoute
     * 
     * @param m HTTP verb
     * @param uri Absolute route to match
     * @param cb Function pointer
     */
    void addRoute(method m, std::string uri, RequestCallbackFunction cb);

    /**
     * @brief Core route/handler mapping method matching absolute URIs
     * 
     * @note Absolute matching takes priority over glob pattern
     * 
     * @see @ref muapp::API::addgRoute
     * 
     * @param m HTTP verb
     * @param uri Absolute route to match
     * @param cb Request handler implementation
     */
    void addRoute(method m, std::string uri, RequestCallbackSharedPtr cb);

    /**
     * @brief Core route/handler mapping methods matching glob patterns on URIs
     * 
     * @note Absolute matching takes priority over glob pattern
     * 
     * @see @ref muapp::API::addRoute
     * @param m HTTP verb
     * @param globuri A glob pattern to match
     * @param cb Function pointer
     */
    void addgRoute(method m, std::string globuri, RequestCallbackFunction cb);

    /**
     * @brief Core route/handler mapping methods matching glob patterns on URIs
     * 
     * @note Absolute matching takes priority over glob pattern
     * 
     * @see @ref muapp::API::addRoute
     * @param m HTTP verb
     * @param globuri A glob pattern to match
     * @param cb Request handler implementation
     */
    void addgRoute(method m, std::string globuri, RequestCallbackSharedPtr cb);

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

/**
 * @brief Mongoose framework wrapper
 * 
 */
class MuApp {
public:
    MuApp();
    ~MuApp();

    /**
     * @brief Register an HTTP server with an API object as handler
     * 
     * @param api 
     * @param port TCP port
     */
    void listen(API * api, unsigned int port);

    /**
     * @brief Register a HTTP server with the given function as handler
     * 
     * @see https://cesanta.com/docs/
     * @param fn Handler
     * @param fn_data Data passed to handler on event
     * @param port TCP port
     */
    void listen(mg_event_handler_t fn, void * fn_data, unsigned int port);

    /**
     * @brief Register an TCP server with the given function as handler
     * 
     * @see https://cesanta.com/docs/
     * @param fn Handler
     * @param fn_data Data passed to handler on event
     * @param port TCP port
     */
    void listenRaw(mg_event_handler_t fn, void * fn_data, unsigned int port);
    void launch(void);
    void synchronize(std::mutex * m);
private:
    std::string listenUrl(unsigned int port);
    std::mutex * synchro = NULL;
    struct mg_mgr mgr;
};
/**
 * @brief <=> app->listen(api, port)
 * 
 * @param api 
 * @param app 
 * @param port 
 */
void attach(API * api, MuApp * app, unsigned int port);

} //namespace muapp
#endif //MUAPP_HEADER