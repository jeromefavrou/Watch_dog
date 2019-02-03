#include "watch_dog.hpp"

Watch_dog::Watch_dog():Server(nullptr)
{
    this->states.as_client=false;
    this->states.quarentine=false;
    this->states.stopping=false;
    this->states.tcp_ip=false;
    this->states.monitor=false;

    this->password="";

    this->devices=std::make_unique<Net_devices>(Net_devices());
    this->devices->load();

    long int q(0);

    for(auto & d : this->devices->get_device())
    {
        if(d.identity.mac_addr=="00:00:00:00:00:00") // <- ignore virtual interface
            continue;

        q+=d.tx.bytes;
    }

    this->debit_Mo_s=static_cast<float>(q)/(1000*1000);

    #ifdef _DEBUG_MOD
    this->devices->display();
    #endif
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

void Watch_dog::update_debit(float time_ms)
{
    float elaps(0);

    long int q(this->debit_Mo_s*(1000*1000)),l(0);
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

        elaps=static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-start).count());
        std::this_thread::sleep_for(std::chrono::duration<float,std::milli>(time_ms-elaps));

        this->debit_Mo_s=r/(static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-start).count())/1000);

        #ifdef _DEBUG_MOD
        std::cout << this->debit_Mo_s << " Mo/s" <<std::endl;
        #endif

        if(this->states.monitor)
        {
            std::stringstream float2str;
            float2str << this->debit_Mo_s;

            std::string deb;
            float2str >> deb;

            this->single_send(deb+std::string("Mo/s"));
        }
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

        this->states.tcp_ip=true;

        return true;
    }
    catch(std::string const & error)
    {
        std::cerr << "init server failed: "<< error << std::endl;

        this->Server->CloseSocket(0);

        return false;
    }
    catch(std::exception const & error)
    {
        std::cerr << "init server failed: "<< error.what() << std::endl;

        this->Server->CloseSocket(0);

        return false;
    }

    std::cerr << "init server failed: ??? error"<< std::endl;

    cmd_unix::notify_send("init server failed: ??? error");

    return false;
}

void Watch_dog::accept_client(void)
{
    while(states.tcp_ip)
    {
        if(!this->states.as_client)
        {
            this->password="";
            this->states.monitor=false;
            this->Server->AcceptClient(0,0);
            this->states.as_client=true;
        }
        //limite l'utilisation de la cpu (ms)
        std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(500));
    }
}

void Watch_dog::main_loop_server(void)
{
    if(this->Server==nullptr || !this->states.tcp_ip)
        return ;

    #ifdef _DEBUG_MOD
    std::clog << "main loop server running" << std::endl;
    #endif // _DEBUG_MOD

    std::thread ac(&Watch_dog::accept_client,this);

    while(this->states.tcp_ip)
    {
        if(this->auth())
        {
            std::stringstream DataBuffer;
            std::string word("");
            Tram RepData;

            if(!this->rcv_data(DataBuffer))
                continue;

            this->CMD(DataBuffer);

        }
        //limite l'utilisation de la cpu (ms)
        std::this_thread::sleep_for(std::chrono::duration<int,std::milli>(200));
    }

    ac.join();

    #ifdef _DEBUG_MOD
    std::clog << "main loop server ending" << std::endl;
    #endif // _DEBUG_MOD
}

bool Watch_dog::rcv_data(std::stringstream & data)
{
    VCHAR BufferReq;

    int length=this->Server->Read<2048>(0,BufferReq);

    //si deconnection du client
    if(length==0)
    {
        #ifdef _DEBUG_MOD
        std::clog<<"Le client à déconnecté"<<std::endl;
        #endif // _DEBUG_MOD

        this->states.as_client=false;

        return false;
    }
    //sinon si requete reçu
    else if(length>0)
    {
        //affichage tram (hexa)
        #ifdef _DEBUG_MOD
        for(auto &i : BufferReq)
            std::clog <<"0x"<<std::hex <<static_cast<int>(i)<<" " ;
        std::clog <<std::dec<< std::endl;
        #endif // _DEBUG_MOD

        //supression des 2 octets de fin de trensmission (tlenet)
        BufferReq.erase(BufferReq.end()-2,BufferReq.end());

        data.clear();

        data << BufferReq.data();

        return true;
    }

    return false;
}

bool Watch_dog::auth(void)
{
    if(!this->states.as_client)
        return false;

    if(this->check_pwd())
        return true;

    std::stringstream DataBuffer;

    this->single_send("Password: ");

    DataBuffer.clear();

    if(!this->rcv_data(DataBuffer))
        return false;

    DataBuffer >> this->password;

    DataBuffer.clear();

    if(!this->check_pwd())
    {
        this->single_send("invalide password");
        return false;
    }
    else
    {
        this->single_send("enter HELP for show cmd possible");
        return true;
    }

    return false;
}

bool Watch_dog::check_pwd(void)
{
    return Extract_one_data<std::string>("pwd_hash")==picosha2::hash256_hex_string(this->password);
}

void Watch_dog::single_send(std::string const & rep)
{
    Tram RepData;

    RepData+="WD> ";
    RepData+=rep;
    RepData+="\n";
    this->Server->Write(0,RepData.get_c_data());
}

void Watch_dog::CMD(std::stringstream & data)
{
    std::string word("");
    data >> word;
    Tram RepData;

    if(word=="HELP")
    {
        std::fstream If("HELP",std::ios::in);

        if(!If)
        {
            this->single_send("help not found");
            return;
        }

        std::string line;

        RepData+="WD> ";

        while(std::getline(If,line))
        {
            RepData+=line;
            RepData+="\n";
        }

        this->Server->Write(0,RepData.get_c_data());
        RepData.clear();

        return;
    }
    else if(word=="MONITOR")
    {
        if(!this->SET<bool>(data,this->states.monitor))
        {
            this->single_send("invalide cmd/action");
            return;
        }
        return;
    }
    else if(word=="PASSWORD")
    {
        if(!this->SET<std::string>(data,this->password))
        {
            this->single_send("invalide cmd/action");
            return;
        }
        std::fstream Of("pwd_hash",std::ios::out);

        if(Of)
            Of << picosha2::hash256_hex_string(this->password);
        else
        {
            this->single_send("password can not be change: hash not found");
            return;
        }

        this->single_send("password has changed");

        return;
    }
    else if(word=="RESTART")
    {

    }
    else if(word=="STOP")
    {

    }
    else if(word=="STATES")
    {
        this->single_send("stopping: "+std::string((this->states.stopping?"true":"false")));
        this->single_send("quarentine: "+std::string((this->states.quarentine?"true":"false")));
        this->single_send("as_client: "+std::string((this->states.as_client?"true":"false")));
        this->single_send("tcp_ip: "+std::string((this->states.tcp_ip?"true":"false")));
        this->single_send("monitor: "+std::string((this->states.monitor?"true":"false")));
    }
    else if(word=="LOG")
    {

    }
    else if(word=="TCP_PORT")
    {

    }
    else if(word=="MAX_FLOW")
    {

    }
    else if(word=="PERIODE_TIME")
    {

    }
    else if(word=="PERIODE_FILTER")
    {

    }
    else if(word=="CONFIG")
    {

    }
}
