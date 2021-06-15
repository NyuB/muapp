#include <iostream>
#include "muapp.hpp"
#include <sstream>

/**
 * @brief Example of a raw function used as callback
 * 
 * @param req 
 * @param res 
 */
void PingSimple(mg_http_message * req, muapp::ResponseSharedPtr res){
    res->send(200, "{\"pong\":42}");
}

/**
 * @brief Example of a callback handler with "memory"
 * 
 */
class PingCount : public muapp::RequestCallback {
public:
    PingCount():cnt(0){};
    virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res) {
        std::stringstream sb;
        sb << "{\"pong:\""<<std::to_string(cnt)<<"}";
        cnt++;
        res->send(200, sb.str());
    }
protected:
private:
    long cnt;
};

int main(int argc, char ** argv){
    unsigned int port = 5555;
    std::cout << "Server setup ..." << std::endl;
    muapp::MuApp app;
    muapp::API * api = (new muapp::API())->useJson();
    muapp::RequestCallbackSharedPtr ping(new PingCount());
    if(argc > 1){
        port = std::atoi(argv[1]);
    }
    api->get("/pingc", ping);
    api->get("/ping",PingSimple);
    app.listen(api, port);
    app.launch();
    return 0;
}
