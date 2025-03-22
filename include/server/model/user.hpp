#ifndef USER_H
#define USER_H
#include <string>
using namespace std;

//匹配User表的ORM类
class User{
public:
    User(int id = 1, string name = "", string pwd = "", string state = ""){
        this->m_id = id;
        this->m_name = name;
        this->m_password = pwd;
        this->m_state = state;
    }

    inline void setId(int id){this->m_id = id;}
    inline void setName(string name){this->m_name = name;}
    inline void setPwd(string pwd){this->m_password = pwd;}
    inline void setState(string state){this->m_state = state;}

    inline int getId(){return this->m_id;}
    inline string getName(){return this->m_name;}
    inline string getPwd(){return this->m_password;}
    inline string getState(){return this->m_state;}

private:
    int m_id;
    string m_name;
    string m_password;
    string m_state;

};


#endif