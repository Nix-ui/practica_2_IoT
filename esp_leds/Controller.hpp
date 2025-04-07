#pragma once
#include "Led.hpp"
template <typename T,typename U>
class Controller{
    public:
  		Controller(){};
        virtual void setAllState(T state)=0;
        virtual void setState(U pin,T tate)=0;
};
