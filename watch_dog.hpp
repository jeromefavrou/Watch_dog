#ifndef WATCH_DOG_HPP_INCLUDED
#define WATCH_DOG_HPP_INCLUDED

#include "net_device.hpp"
#include "serveur.cpp" //passer hpp
#include <thread>
#include <memory>

class Watch_dog
{
public:
    Watch_dog(void);

    bool init_server(uint32_t const port);
    void accecpt_client(void);//-> in main_loop_server
    void main_loop_server(void);

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

    long int debit_Mo_s;

    State states;
    std::unique_ptr<Net_devices> devices;
    std::unique_ptr<CSocketTCPServeur> Server;
};

#endif // WATCH_DOG_HPP_INCLUDED
