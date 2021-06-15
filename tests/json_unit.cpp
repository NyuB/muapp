#include "gmock/gmock.h"
#include <cesanta/frozen.h>


TEST(GTestSetup, Json){
    //Compiling ok
}

TEST(JsonPartialParse, NoChangeIfAbsent){
    std::string json("{i : 15, b : true}");
    int i = 0;
    bool b = false;
    int c = 42;
    int r = json_scanf(json.c_str(), json.length(), "{i:%d, b:%B, c:%d}", &i, &b, &c);
    ASSERT_EQ(r, 2);
    ASSERT_EQ(i, 15);
    ASSERT_TRUE(b);
    ASSERT_EQ(c, 42);
}