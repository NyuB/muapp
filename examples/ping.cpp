/**
 * @file ping.cpp
 * @author NyuB
 * @brief Simple ping server, with illustrations of both a raw function (at GET(/ping) ) and a class (at GET(/pingc) )used as callbacks
 * @note The object callback allows implementing a "memory" behavior, here the number of ping requests receive
 * @version 0.1
 * @date 2021-06-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
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
    muapp::API * api = (new muapp::API())->useJson();//set and expect header content-type:application/json
    if(argc > 1){
        port = std::atoi(argv[1]);
    }
    //Register function directly
    api->get("/ping",PingSimple);

    //Instanciate handler then register
    muapp::RequestCallbackSharedPtr ping(new PingCount());
    api->get("/pingc", ping);

    //Register API on listening port
    app.listen(api, port);

    //Starts blocking infinite event loo
    app.launch();
    return 0;
}
