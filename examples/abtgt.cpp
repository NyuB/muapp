#include <iostream>
#include "muapp.hpp"
#include <sstream>
#include <thread>
#include <mutex>
std::mutex gm;
void PingSimple(mg_http_message * req, muapp::ResponseSharedPtr res){
    res->status(200);
    std::cout << "Exiting Ping" << std::endl;
}
void ParallelPing(mg_http_message * req, muapp::ResponseSharedPtr res){
    res->synchronize(&gm);
    std::thread t(PingSimple, req, res);
    t.detach();
}

int main(int argc, char ** argv){
    unsigned int port = 5555;
    std::cout << "Server setup ..." << std::endl;
    muapp::MuApp app;
    muapp::API * api = (new muapp::API());
    if(argc > 1){
        port = std::atoi(argv[1]);
    }
    api->get("/",ParallelPing);
    app.synchronize(&gm);
    app.listen(api, port);
    app.launch();
    return 0;
}
