#include "tc_epoll_server.h"


int main(){
    auto  vNetThread =  new tars::NetThread();
    std::string ip = "127.0.0.1";
    int port = 9877;
    vNetThread->bind(ip,port);
    vNetThread->createEpoll(0);
    vNetThread->run();

    delete vNetThread;
    return 0;
}