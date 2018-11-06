#include "watch_dog.hpp"

Watch_dog::Watch_dog():Server(nullptr)
{
    this->devices=std::make_unique<Net_devices>(Net_devices());
    this->devices->load();
    this->debit_Mo_s=0;
    //this->devices->display();
}

Watch_dog::~Watch_dog()
{
    this->devices.release();

    this->Server->CloseSocket(0);
    this->Server.release();

    if(this->devices!=nullptr)
        this->devices=nullptr;

    if(this->Server!=nullptr)
        this->Server=nullptr;
}


bool Watch_dog::init_server(uint32_t const port)
{
    try
    {
        this->Server=std::make_unique<CSocketTCPServeur>(CSocketTCPServeur());

        this->Server->NewSocket(0);

        this->Server->BindServeur(0,INADDR_ANY,port);

        this->Server->Listen(0,1);

        return true;
    }
    catch(std::string const & error)
    {
        std::cerr << "init server failed: "<< error << std::endl;

        this->Server->CloseSocket(0);

        this->Server.release();

        return false;
    }
    catch(std::exception const & error)
    {
        std::cerr << "init server failed: "<< error.what() << std::endl;

        this->Server->CloseSocket(0);

        this->Server.release();

        return false;
    }

    std::cerr << "init server failed: ??? error"<< std::endl;

    return false;
}

void Watch_dog::main_loop_server(void)
{
    std::clog << "main loop server running" << std::endl;


    std::clog << "main loop server ending" << std::endl;
}
