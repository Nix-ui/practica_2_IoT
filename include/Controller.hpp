#pragma once
#include "Led.hpp"
class Controller{
    public:
  		Controller(){};
        virtual void setAllState(Led::State)=0;
        virtual void setState(byte,Led::State)=0;
};
