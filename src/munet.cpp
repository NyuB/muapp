#include "munet.hpp"
namespace munet {

/*
>>>>>>>>>>>>>>>>>>>>
class MgHttpForward
>>>>>>>>>>>>>>>>>>>>
*/
void minimalHandler(mg_connection * c, int ev, void * ev_data, void * fn_data){
    switch(ev) {
        case MG_EV_ACCEPT:
        case MG_EV_CLOSE:
        case MG_EV_CONNECT:
        case MG_EV_ERROR:
        case MG_EV_HTTP_CHUNK:
        case MG_EV_HTTP_MSG:
        case MG_EV_READ:
        case MG_EV_POLL:
        case MG_EV_RESOLVE:
        case MG_EV_USER:
        case MG_EV_WRITE:
        default:
            return;
    }
}
mg_connection * MgHttpForward::forward(mg_http_message const& data, mg_connection * const& clt){
    if(forward_c == NULL){
        struct mg_str * host = mg_http_get_header((mg_http_message*)&data, "Host");
        if(host==NULL)return NULL;
        forward_c = mg_http_connect(clt->mgr, dns->resolve(mgs2s(host)).c_str(), minimalHandler, NULL);
    }
    return forward_c;
}

/*
<<<<<<<<<<<<<<<<<<<<
class MgHttpForward
<<<<<<<<<<<<<<<<<<<<
*/

}//namespace munet