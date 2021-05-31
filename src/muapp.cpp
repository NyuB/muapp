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


RequestCallback::RequestCallback(){};
RequestCallback::~RequestCallback(){};
void RequestCallback::operator()(mg_http_message * req, ResponseSharedPtr res) {
    call(req, res);
}

FunctionPointerRequestCallback::FunctionPointerRequestCallback(void cb_(mg_http_message *, ResponseSharedPtr res)):cb(cb_){};
FunctionPointerRequestCallback::~FunctionPointerRequestCallback(){};
void FunctionPointerRequestCallback::call(mg_http_message * req, ResponseSharedPtr res) {
    cb(req,res);
}

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
        auto find = api->routes[m].find(uri);
        if(find!=api->routes[m].end())api->routes[readMethod(req)][mgs2s(req->uri)]->call(req, r);
        else{
            bool match = false;
            for(auto it = api->globRoutes[m].begin();it!=api->globRoutes[m].end();it++){
                if(mg_http_match_uri(req, it->first.c_str())){
                    it->second->call(req,r);
                    match = true;
                    break;
                }
            }
            if(!match){
                mg_http_reply(c, 404, NULL, "Not Found");
            }
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
void API::addRoute(method m, std::string uri, void cb(mg_http_message *, ResponseSharedPtr)){
    routes[m][uri] = std::make_shared<FunctionPointerRequestCallback>(cb);
}
void API::addRoute(method m, std::string uri, RequestCallbackSharedPtr cb){
    routes[m][uri] = cb;
}
void API::get(std::string uri, void cb(mg_http_message *, ResponseSharedPtr)){
    addRoute(GET, uri, cb);
}
void API::get(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(GET, uri, cb);
}
void API::put(std::string uri, void cb(mg_http_message *, ResponseSharedPtr)){
    addRoute(PUT, uri, cb);
}
void API::put(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(PUT, uri, cb);
}
void API::post(std::string uri, void cb(mg_http_message *, ResponseSharedPtr)){
    addRoute(POST, uri, cb);
}
void API::post(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(POST, uri, cb);
}
void API::del(std::string uri, void cb(mg_http_message *, ResponseSharedPtr)){
    addRoute(DEL, uri, cb);
}  
void API::del(std::string uri, RequestCallbackSharedPtr cb){
    addRoute(DEL, uri, cb);
}
MuApp::MuApp(){
    mg_mgr_init(&mgr);
}
MuApp::~MuApp(){
    mg_mgr_free(&mgr);
}
void MuApp::listen(API * api, unsigned int port){
    mg_http_listen(&mgr, ("0.0.0.0:"+std::to_string(port)).c_str(), API::receive, api);
}
void MuApp::launch(void){
    for(;;) mg_mgr_poll(&mgr, 1000);
}
}//namespace muapp