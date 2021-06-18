/**
 * @file alphabet.cpp
 * @author NyuB
 * @brief Example of a minimal REST resource controller. The Alphabet resource stores names at the paths /name/c where c is a character
 * Follows an usual REST behavior, GET/DEL(name/c) returns/delete the name at the given character, POST/PUT(name/) create/update/delete a name according to the json body
 * @note Usage of a macro definition for a resource accessor MacroPing registered for GET(/ping) and equivalent to GET(/name/p)
 * @version 0.1
 * @date 2021-06-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "muapp.hpp"
#include "cesanta/frozen.h"
#include <map>

/**
 * @brief Resource class
 * 
 */
class Alphabet {
public:
    /**
     * @brief Construct a new Alphabet object and fills some default character
     * 
     */
    Alphabet(){
        alphabet['a'] = "Alan";
        alphabet['b'] = "Brice";
        alphabet['c'] = "Carla";
        alphabet['d'] = "Diane";
        alphabet['p'] = "Pong";
    }
    ~Alphabet(){};
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //REST logic implementation in resource class
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    std::string get(char c) const {
        auto it = alphabet.find(c);
        if(it == alphabet.end())return "";
        return it->second;
    }
    bool post(char c, std::string name){
        auto it = alphabet.find(c);
        if(it != alphabet.end())return false;
        alphabet[c] = name;
        return true;
    }
    bool put(char c, std::string name){
        auto it  = alphabet.find(c);
        if(it == alphabet.end()){
            return false;
        }
        else{
            alphabet[c] = name;
            return true;
        }
    }
    bool del(char c){
        auto it  = alphabet.find(c);
        if(it == alphabet.end())return false;
        alphabet.erase(it);
        return true;
    }
private:
    std::map<char, std::string> alphabet;
};

/**
 * @brief REST request body class
 * 
 */
class LetterName {
public:
    LetterName(){};
    char letter;
    std::string name;
    /**
     * @brief Requirement to use LetterName as type parameter for a muapp::JsonRequestCallback
     * 
     * @param s original json string
     * @return LetterName the parsed json object
     */
    static LetterName fromJson(std::string s){
        char * name = NULL;
        LetterName res;
        json_scanf(s.c_str(), s.length(), "{letter:%c, name:%Q}", &res.letter, &name);
        if(name!=NULL){
            res.name = std::string(name);
            free(name);
        }
        return res;
    }
};

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//REST web interface in request handler classes
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class AlphaGet : public muapp::RequestCallback {
public:
    AlphaGet(std::shared_ptr<const Alphabet> resource_):resource(resource_){}
    ~AlphaGet(){}
    virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res) override{
        std::string uri = mgs2s(req->uri);
        char c = uri[uri.length()-1];
        std::string name = resource->get(c);
        if(name!=""){
            res->send(200, "{\"name\":\""+name+"\"}");
        }
        else{
            res->send(404, "");
        }
    }
private:
    std::shared_ptr<const Alphabet> resource;
};



class AlphaPost : public muapp::JsonRequestCallback<LetterName> {
public:
    AlphaPost(std::shared_ptr<Alphabet> resource_):resource(resource_){}
    ~AlphaPost(){}
    virtual void jcb(mg_http_message * req, LetterName const& ln, muapp::ResponseSharedPtr res) override{
        bool success = resource->post(ln.letter, ln.name);
        if(success){
            res->send(200,"/name/"+ln.letter);
        }
        else{
            res->send(400, "");
        }
    }
private:
    std::shared_ptr<Alphabet> resource;
};

class AlphaPut : public muapp::JsonRequestCallback<LetterName> {
public:
    AlphaPut(std::shared_ptr<Alphabet> resource_):resource(resource_){}
    ~AlphaPut(){}
    virtual void jcb(mg_http_message * req, LetterName const& ln, muapp::ResponseSharedPtr res) override{
        bool success = resource->put(ln.letter, ln.name);
        if(success){
            res->send(200,"/name/"+ln.letter);
        }
        else{
            res->send(400, "");
        }
    }
private:
    std::shared_ptr<Alphabet> resource;
};

class AlphaDel : public muapp::RequestCallback {
public:
    AlphaDel(std::shared_ptr<Alphabet> resource_):resource(resource_){}
    ~AlphaDel(){}
    virtual void call(mg_http_message * req, muapp::ResponseSharedPtr res) override{
        std::string uri = mgs2s(req->uri);
        char c = uri[uri.length()-1];
        bool success = resource->del(c);
        if(success){
            res->send(200,"");
        }
        else{
            res->send(400, "");
        }
    }
private:
    std::shared_ptr<Alphabet> resource;
};

/**
 * @brief Define a minimal class MacroPing with a sharedpointer to a const Alphabet object as 'resource' field
 * 
 */
MUAPP_RSC_READ(Alphabet, MacroPing, {
    std::string name = resource->get('p');
    if(name!=""){
        res->send(200, "{\"name\":\""+name+"\"}");
    }
    else{
        res->send(404, "");
    }
});

int main(int argc, char ** argv){
    std::cout << "Launching Alphabet App" << std::endl;
    muapp::API api;
    muapp::MuApp app;

    //Create the main resource
    std::shared_ptr<Alphabet> alpha = std::make_shared<Alphabet>();

    //Link handlers to resource
    muapp::RequestCallbackSharedPtr alphaget(new AlphaGet(alpha));
    muapp::RequestCallbackSharedPtr alphapost(new AlphaPost(alpha));
    muapp::RequestCallbackSharedPtr alphaput(new AlphaPut(alpha));
    muapp::RequestCallbackSharedPtr alphadel(new AlphaDel(alpha));
    muapp::RequestCallbackSharedPtr macroping(new MacroPing(alpha));//Use of MacroPing defined in function

    //Map routes and methods
    api.gget("/name/?", alphaget);
    api.gdel("/name/?", alphadel);
    api.post("/name/", alphapost);
    api.post("/name", alphapost);//Multiple routes to the same handler, here to handle the '/' or ' ' ambiguity
    api.put("/name/", alphaput);
    api.put("/name", alphaput);//Multiple routes to the same handler, here to handle the '/' or ' ' ambiguity
    api.get("/ping/", macroping);//Multiple routes to the same handler, here to handle the '/' or ' ' ambiguity
    api.get("/ping", macroping);

    //Register API to a port
    app.listen(&api, 8888);

    //Starts blocking infinite event loop
    app.launch();
}