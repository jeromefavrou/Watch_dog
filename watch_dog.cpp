#include "watch_dog.hpp"

Watch_dog::Watch_dog()
{
    devices=std::make_unique<Net_devices>(Net_devices());
    this->devices->load();
    //this->devices->display();
}

Watch_dog::~Watch_dog()
{
    if(devices!=nullptr)
        devices=nullptr;
}
