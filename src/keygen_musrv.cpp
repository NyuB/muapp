#include <iostream>
#include "muapp.hpp"
int main(int argc, char ** argv){
    std::cout << "Server setup ..." << std::endl;
    muapp::MuApp app;
    muapp::API * api = (new muapp::API())->useJson();
    api->get("/ping",[](mg_http_message * req, muapp::ResponseSharedPtr res) {
        res->send(200, "{pong:42}");
    });
    api->post("/credential", [](mg_http_message * req, muapp::ResponseSharedPtr res) {
        res->send(200, "{key:\"abc\"}");
    });
    api->post("/policy", [](mg_http_message * req, muapp::ResponseSharedPtr res) {
        res->send(200, "{policy:\"A or (B and T=5)\"}");
    });
    api->post("/decrypt", [](mg_http_message * req, muapp::ResponseSharedPtr res) {
        res->send(200, "{data:\"Clear\"}");
    });
    api->post("/encrypt", [](mg_http_message * req, muapp::ResponseSharedPtr res) {
        res->send(200, "{data:\"!â#ls8\"}");
    });
    app.listen(api, 5555);
    app.launch();
    std::cout << "Server closed" << std::endl;
    return 0;
}
