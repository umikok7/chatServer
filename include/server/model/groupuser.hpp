#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

//群组用户，多了一个role角色信息，从User类直接继承，复用User其他信息

class GroupUser : public User{
public:
    inline void setRole(string role){ this->m_role = role; }
    inline string getRole(){ return this->m_role; }

private:
    string m_role;
};




#endif