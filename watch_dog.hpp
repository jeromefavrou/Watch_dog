#ifndef WATCH_DOG_HPP_INCLUDED
#define WATCH_DOG_HPP_INCLUDED

#define _DEBUG_MOD

#include "net_device.hpp"
#include "serveur.cpp" //passer hpp
#include "picosha2.hpp"
#include <thread>
#include <memory>
#include <sstream>
#include <fstream>


class Watch_dog
{
public:
    Watch_dog(void);

    bool init_server(uint32_t const port);
    void main_loop_server(void);
    void update_debit(float time_ms);

    void watch(void);

    ~Watch_dog(void);

private:
    struct State
    {
        bool stopping;
        bool quarentine;
        bool as_client;
        bool tcp_ip;
        bool monitor;
    };

    void server_client(void);
    void accept_client(void);
    bool auth(void);
    bool rcv_data(std::stringstream & data);
    bool check_pwd(void);
    void single_send(std::string const & rep);

    void restart(void);
    void stop(void);
    void quarentine(void);
    void log(void);

    void CMD(std::stringstream & data);

    template<class T> bool SET(std::stringstream & data,T & buff)
    {
        std::string w("");
        data >> w;
        if(w=="SET")
        {
            data >> buff;
            return true;
        }

        return false;
    }

    float debit_Mo_s;

    std::string password;
    State states;
    std::unique_ptr<Net_devices> devices;
    std::unique_ptr<CSocketTCPServeur> Server;
};

#endif // WATCH_DOG_HPP_INCLUDED
