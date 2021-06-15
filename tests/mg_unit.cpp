#include "gmock/gmock.h"
#include <cesanta/frozen.h>
#include <cesanta/mongoose.h>
#include <cstring>

const char * request = "POST http://localhost:5555/api HTTP/1.1\r\nHost: localhost:5555\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}";
const char * only_head_nobody = "POST http://localhost:5555/api HTTP/1.1\r\nHost: localhost:5555\r\nContent-Type: application/json\r\nContent-Length: 0\r\n\r\n";
const char * only_head_missing_body = "POST http://localhost:5555/api HTTP/1.1\r\nHost: localhost:5555\r\nContent-Type: application/json\r\nContent-Length: 1024\r\n\r\n";
//126 bytes request + 2bytes body

TEST(GTestSetup, Mongoose){
    //Compiling ok
}
TEST(MongooseHttp, RequestParsing){
    struct mg_http_message req;
    int r = mg_http_parse(request, strlen(request), &req);
    EXPECT_GT(r, 0) << "Request parsing should be successful";
    EXPECT_EQ(strlen(request), req.message.len) << "Entire request should be parsed";
    struct mg_http_message head;
    r = mg_http_parse(only_head_nobody, strlen(only_head_nobody), &head);
    EXPECT_EQ(strlen(only_head_nobody), r) << "Parsing shoudl be valid";
    EXPECT_EQ(strlen(only_head_nobody), head.message.len) << "Message is request";
    EXPECT_EQ(0, head.body.len) << "Body should be parsed as 0 byte";
    EXPECT_EQ(strlen(only_head_nobody), head.head.len) << "Entire request part should be parsed";
    struct mg_http_message missing;
    r = mg_http_parse(only_head_missing_body, strlen(only_head_missing_body), &missing);
    EXPECT_EQ(strlen(only_head_missing_body), r) << "Parsing should be valid";
    EXPECT_LT(strlen(only_head_missing_body), missing.message.len) << "Message is request";
    EXPECT_LT(0, missing.body.len) << "Body should be parsed as some bytes";
    EXPECT_EQ(strlen(only_head_missing_body), missing.head.len) << "Entire request part should be parsed";
}

