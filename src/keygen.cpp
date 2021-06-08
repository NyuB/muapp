#include <iostream>
#include "muapp.hpp"
#include "mujson.hpp"
#include "mubyte.hpp"
#include <sstream>
#define CPABE long //TODO replace with actual type

using spair = std::pair<std::string, std::string>;
spair KEYGEN_PLACEHOLDER_RSA(void){
    return spair("RSA KEY","RSA_PUB");
}

std::string KEYGEN_PLACEHOLDER_ABE(std::string attr){
    return std::string("KEY FOR ["+attr+"]");
}

class KeyGenRCB : public muapp::RequestCallback {
public:
    KeyGenRCB(CPABE * cpabe_):cpabe(cpabe_){

    };
    virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res) {
        char * type = NULL;
        char * attributes = NULL;
        std::cout << mgs2s(req->body) << std::endl;
        json_scanf(req->body.ptr, req->body.len, "{type:%Q, attributes:%Q}", &type, &attributes);
        if(type != NULL){
            std::string ts(type);
            if(ts == "rsa"){
                spair kp = KEYGEN_PLACEHOLDER_RSA();
                std::stringstream sb;
                sb << "{privateKey:" << '"' << kp.first << '"' << ",publicKey:" << '"' << kp.second << '"' << "}";
                res->send(200, sb.str());
            }
            else if(ts == "cp-abe"){
                if(attributes != NULL){
                    std::stringstream sb;
                    std::string k = KEYGEN_PLACEHOLDER_ABE(attributes);
                    sb << "{privateKey:" << '"' << k << '"' << "}";
                    res->send(200, sb.str());
                }
                else{
                    res->send(400, "Attribute list required for CP-ABE Key");
                }
            }
            else {
                res->send(400, "Unsupported key type");
            }
        }
        else{
            res->send(400, "Key type  is mandatory");
        }
        if(type != NULL)free(type);
        if(attributes != NULL)free(attributes);
    }
protected:
private:
    CPABE * cpabe;
};

int main(int argc, char ** argv){
    std::cout << "Server setup ..." << std::endl;
    muapp::MuApp app;
    muapp::API * api = (new muapp::API())->useJson();
    CPABE cpabe;
    muapp::RequestCallbackSharedPtr keygen(new KeyGenRCB(&cpabe));
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
    api->post("/key", keygen);
    app.listen(api, 5555);
    app.launch();
    std::cout << "Server closed" << std::endl;
    return 0;
}
