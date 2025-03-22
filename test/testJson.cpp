#include "../include/json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <map>
#include <vector>
#include <string>
using namespace std;

//json序列化事例
string func1(){
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello, what are you doing now?";

    string sendBuf = js.dump();

    // cout << js << endl;

    // cout << sendBuf.c_str() << endl;
    return sendBuf;
}

string func2(){
    vector<int> vec;
    json js;
    for(int i = 0; i < 3; i++){
        vec.push_back(i);
    }
    js["list"] = vec;

    map<int, string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});
    m.insert({3,"泰山"});
    js["path"] = m;

    // cout<<js<<endl;

    // string senBuf = js.dump();
    // cout << senBuf << endl;

    return js.dump();
}




int main(){
    //func1();
    //func2();
    string recvBuf = func2();
    json jsBuf = json::parse(recvBuf);
    // cout << jsBuf["msg_type"] << endl;
    // cout << jsBuf["from"] << endl;
    // cout << jsBuf["to"] << endl;
    // cout << jsBuf["msg"] << endl;

    vector<int> vec = jsBuf["list"];
    //map<int, string> 
    for(auto &v : vec){
        cout << v << " ";
    }
    cout << endl;

    map<int, string> m = jsBuf["path"];
    for(auto &p : m){
        cout << p.first << " " << p.second << endl;
    }
    cout << endl;

    return 0;
}
