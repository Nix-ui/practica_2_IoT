#pragma once

class Controller{
    public:
  		Controller(){};
        virtual void setAllState(byte)=0;
        virtual void setState()=0;
};
