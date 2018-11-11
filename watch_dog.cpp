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

void Watch_dog::update_debit(void)
{
    double time_ms(1000),elaps(0);

    long int q(0),l(0);
    float r(0);

    std::chrono::time_point<std::chrono::system_clock> start;

    while(true)
    {
        start = std::chrono::system_clock::now();

        l=q;
        q=0;

        this->devices->update();
        for(auto & d : this->devices->get_device())
        {
            if(d.identity.mac_addr=="00:00:00:00:00:00") // <- ignore virtual interface
                continue;

            q+=d.tx.bytes;
        }

        r=static_cast<float>(q-l)/(1000*1000);

        elaps=std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-start).count();
        std::this_thread::sleep_for(std::chrono::duration<double,std::milli>(time_ms-elaps));

        this->debit_Mo_s=r/(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-start).count());

        std::cout << this->debit_Mo_s << " Mo/s" <<std::endl;
    }
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
    if(this->Server==nullptr)
        return ;

    std::clog << "main loop server running" << std::endl;


    std::clog << "main loop server ending" << std::endl;
}
