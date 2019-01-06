#include "watch_dog.hpp"
/// net device


int main()
{
    uint32_t const server_port(6547);

    Watch_dog WD;

    if(!WD.init_server(server_port))
        if(!WD.init_server(server_port+1))//second try on other port
            return -1;

    std::thread mls(&Watch_dog::main_loop_server,&WD);
    std::thread ud(&Watch_dog::update_debit,&WD,100);

    ud.join();
    mls.join();

    return 0;
}
