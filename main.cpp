#include "watch_dog.hpp"

/// net device


int main()
{
    Watch_dog WD;

    if(!WD.init_server(6547))
        return -1;

    std::thread mls(&Watch_dog::main_loop_server,&WD);

    mls.join();

    //gnome notification exempl que si GUI demarrer (warning service)
    //notify-send -u critical -i logo.png "Tux-planet" "www.tux-planet.fr" -t 10

    return 0;
}
