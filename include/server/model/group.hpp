#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

//User表的ORM类
class Group{
public:
    Group(int id = -1, string name = "", string desc = ""){
        this->m_id = id;
        this->m_name = name;
        this->m_desc = desc;
    }

    inline int getID(){return this->m_id;}
    inline string getName(){return this->m_name;}
    inline string getDesc(){return this->m_desc;}
    inline vector<GroupUser> &getUsers(){return this->users;}

    inline void setId(int id){this->m_id = id;}
    inline void setName(string name){this->m_name = name;}
    inline void setDesc(string desc){this->m_desc = desc;}

private:
    int m_id;
    string m_name;
    string m_desc;
    vector<GroupUser> users;

};



#endif