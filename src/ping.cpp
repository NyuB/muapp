#include <iostream>
#include "muapp.hpp"
#include "mujson.hpp"
#include "mubyte.hpp"
int main(int argc, char ** argv){
    std::cout << "Server setup ..." << std::endl;
    muapp::MuApp app;
    muapp::API * api = (new muapp::API())->useJson();
    api->get("/ping",[](mg_http_message * req, muapp::ResponseSharedPtr res) {
        mg_http_header * headers = req->headers;
        int i=0;
        std::cout << "Headers : " << MG_MAX_HTTP_HEADERS << " " << (sizeof(req->headers)) <<std::endl;;
        while(headers->name.len!=0){
            std::cout << "Header ["<<i<<"] <"<<mgs2s(headers->name)<<"> <"<<mgs2s(headers->value)<<">"<<std::endl;
            i++;
            headers++;
        }
        res->send(200, "{pong:42}");
    });
    app.listen(api, 5555);
    app.launch();
    std::cout << "Server closed" << std::endl;
    return 0;
}
