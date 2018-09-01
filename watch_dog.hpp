#ifndef WATCH_DOG_HPP_INCLUDED
#define WATCH_DOG_HPP_INCLUDED

#include "net_device.hpp"
#include <thread>
#include <memory>

class Watch_dog
{
public:
    Watch_dog(void);

    void init_server(void);
    void main_server(void);

    void update_debit(void);
    void watch(void);

    ~Watch_dog(void);

private:
    struct State
    {
        bool stopping:1;
        bool quarentine:1;
    };

    void server_client(void);
    void restart(void);
    void stop(void);
    void quarentine(void);
    void log(void);

    State states;
    std::unique_ptr<Net_devices> devices;
};

#endif // WATCH_DOG_HPP_INCLUDED
