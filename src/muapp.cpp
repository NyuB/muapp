#include "muapp.hpp"
namespace muapp {

method readMethod(mg_http_message * req){
    std::string m = mgs2s(req->method);
    if(m=="GET")return GET;
    if(m=="PUT")return PUT;
    if(m=="POST")return POST;
    if(m=="DELETE")return DEL;
    return NONE;
}

/* 
>>>>>>>>>>>>>>
class Response
>>>>>>>>>>>>>>
*/
Response::Response(struct mg_connection * c_):c(c_){}
Response::~Response(){
    reply();
}
Response * Response::status(unsigned int status){
    statuscode = status;
    return this;
}
Response * Response::header(std::string const& key, std::string const& value){
    headers[key]=value;
    return this;
}
Response * Response::write(std::string const& data){
    body << data;
    return this;
}
void Response::send(unsigned int status, std::string const& data){
    statuscode = status;
    body << data;
}

void Response::reply(){
    std::stringstream sb;
    std::string b = body.str();
    for(auto it = headers.begin();it != headers.end();it++){
        sb << it->first << ": " << it->second << "\r\n";
    }
    mg_http_reply(c, statuscode, sb.str().c_str(), b.c_str());
    c->is_draining = true;
}
/* 
<<<<<<<<<<<<<<
class Response
<<<<<<<<<<<<<<
*/

/* 
>>>>>>>>>>>>>>>>>>>>>>
class RequestCallback
>>>>>>>>>>>>>>>>>>>>>>
*/

RequestCallback::RequestCallback(){};
RequestCallback::~RequestCallback(){};
void RequestCallback::operator()(mg_http_message * req, ResponseSharedPtr res) {
    call(req, res);
}

FunctionPointerRequestCallback::FunctionPointerRequestCallback(RequestCallbackFunction cb_):cb(cb_){};
FunctionPointerRequestCallback::FunctionPointerRequestCallback(const RequestCallbackFunction& f):cb(f){};
FunctionPointerRequestCallback::~FunctionPointerRequestCallback(){};
void FunctionPointerRequestCallback::call(mg_http_message * req, ResponseSharedPtr res) {
    cb(req,res);
}
/* 
<<<<<<<<<<<<<<<<<<<<<<
class RequestCallback
<<<<<<<<<<<<<<<<<<<<<<
*/

/* 
>>>>>>>>>>>>>>>>
class API
>>>>>>>>>>>>>>>>
*/

API::API(){
    routes[GET] = std::map<std::string, RequestCallbackSharedPtr>();
    routes[PUT] = std::map<std::string, RequestCallbackSharedPtr>();
    routes[POST] = std::map<std::string, RequestCallbackSharedPtr>();
    routes[DEL] = std::map<std::string, RequestCallbackSharedPtr>();
    globRoutes[GET] = std::map<std::string, RequestCallbackSharedPtr>();
    globRoutes[PUT] = std::map<std::string, RequestCallbackSharedPtr>();
    globRoutes[POST] = std::map<std::string, RequestCallbackSharedPtr>();
    globRoutes[DEL] = std::map<std::string, RequestCallbackSharedPtr>();
};
API::~API(){};
void API::receive(struct mg_connection * c, int ev, void *ev_data, void *fn_data){
    if(ev == MG_EV_HTTP_MSG){
        mg_http_message * req = (mg_http_message*)ev_data;
        API * api = (API*)fn_data;
        ResponseSharedPtr r = std::make_shared<Response>(c);
        if(api->json)r->header("Content-Type", "application/json");
        std::string uri = mgs2s(req->uri);
        method m = readMethod(req);
        bool match = false;
        auto find = api->routes[m].find(uri);
        if(find!=api->routes[m].end()){
            api->routes[readMethod(req)][mgs2s(req->uri)]->call(req, r);
            match=true;
        }
        else{
            for(auto it = api->globRoutes[m].begin();it!=api->globRoutes[m].end();it++){
                if(mg_http_match_uri(req, it->first.c_str())){
                    it->second->call(req, r);
                    match = true;
                    break;
                }
            }
        }
        if(!match){
            r->send(404, "Not Found");
        }
    }
}
API * API::useJson(void){
    json = true;
    return this;
}
API * API::useJson(bool b){
    json = b;
    return this;
}
void API::addRoute(method m, std::string uri, RequestCallbackFunction cb){
    routes[m][uri] = std::make_shared<FunctionPointerRequestCallback>(cb);
}
void API::addRoute(method m, std::string uri, RequestCallbackSharedPtr cb){
    routes[m][uri] = cb;
}

void API::addgRoute(method m, std::string uri, RequestCallbackFunction cb){
    globRoutes[m][uri] = std::make_shared<FunctionPointerRequestCallback>(cb);
}
void API::addgRoute(method m, std::string uri, RequestCallbackSharedPtr cb){
    globRoutes[m][uri] = cb;
}

void API::get(std::string uri, RequestCallbackFunction cb){
    addRoute(GET, uri, cb);
}
void API::get(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(GET, uri, cb);
}

void API::put(std::string uri, RequestCallbackFunction cb){
    addRoute(PUT, uri, cb);
}
void API::put(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(PUT, uri, cb);
}

void API::post(std::string uri, RequestCallbackFunction cb){
    addRoute(POST, uri, cb);
}
void API::post(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(POST, uri, cb);
}

void API::del(std::string uri, RequestCallbackFunction cb){
    addRoute(DEL, uri, cb);
}  
void API::del(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(DEL, uri, cb);
}

void API::gget(std::string uri, RequestCallbackFunction cb){
    addgRoute(GET, uri, cb);
}
void API::gget(std::string uri, RequestCallbackSharedPtr cb){
    addgRoute(GET, uri, cb);
}

void API::gput(std::string uri, RequestCallbackFunction cb){
    addgRoute(PUT, uri, cb);
}
void API::gput(std::string uri, RequestCallbackSharedPtr cb){
    addgRoute(PUT, uri, cb);
}

void API::gpost(std::string uri, RequestCallbackFunction cb){
    addgRoute(POST, uri, cb);
}
void API::gpost(std::string uri, RequestCallbackSharedPtr cb){
    addgRoute(POST, uri, cb);
}

void API::gdel(std::string uri, RequestCallbackFunction cb){
    addgRoute(DEL, uri, cb);
}  
void API::gdel(std::string uri, RequestCallbackSharedPtr cb){
    addgRoute(DEL, uri, cb);
}

/* 
>>>>>>>>>>>>>>>>
class MuApp
>>>>>>>>>>>>>>>>
*/

MuApp::MuApp(){
    mg_mgr_init(&mgr);
}
MuApp::~MuApp(){
    mg_mgr_free(&mgr);
}
void MuApp::listen(API * api, unsigned int port){
    mg_connection * c = mg_http_listen(&mgr, listenUrl(port).c_str(), API::receive, api);
}
void MuApp::listen(mg_event_handler_t fn, void * fn_data, unsigned int port){
    mg_http_listen(&mgr, listenUrl(port).c_str(), fn, fn_data);
}
void MuApp::listenRaw(mg_event_handler_t fn, void * fn_data, unsigned int port){
    mg_listen(&mgr, listenUrl(port).c_str(), fn, fn_data);
}
void MuApp::launch(void){
    for(;;) mg_mgr_poll(&mgr, 1000);
}
std::string MuApp::listenUrl(unsigned int port){
    return "0.0.0.0:"+std::to_string(port);
}

void attach(API * api, MuApp * app, unsigned int port){
    app->listen(api, port);
}
/* 
<<<<<<<<<<<<<<<<
class MuApp
<<<<<<<<<<<<<<<<
*/

}//namespace muapp