#pragma

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "tc_epoller.h"
namespace tars
{

    /*
    负责网络的连接与释放
    */
    class NetThread
    {
    public:
        int bind(std::string& ip, int& port);
        
        static void parseAddr(const std::string &sAddr, struct in_addr &stAddr);

        void createEpoll(uint32_t iIndex = 0);

        bool accept(int fd);
        //运行服务器
        //这里面的业务逻辑是不停的监听来的事件并处理
        void run();


        /// @brief 负责：1.读事件并组织准备回写准备工作
        /// @param ev 发生事件epoll_event
        void processNet(const epoll_event &ev);

        /// @brief 函数应该是负责发回响应？？？
        void processPipe(); 

        enum
        {
            ET_LISTEN = 1,
            ET_CLOSE = 2,
            ET_NOTIFY = 3,
            ET_NET = 0,
        };

        struct
        {
            std::string response;
            uint32_t uid;
        } _response;

    private:
        int m_shutdown_sock;
        int m_notify_sock;

        int m_sock;

        int m_ifd;

        TC_Epoller m_epoller;

        std::string m_recvbuffer;

        std::string m_response;
        //保存connectid ，key:value --> id:fd
        std::map<int, int> m_listen_connect_id;

        std::list<uint32_t> m_free;

        volatile size_t m_free_size;
        

    public:
        /// @brief //建立用于通知和关闭的m_shutdown_sock   m_notify_sock
        NetThread(/* args */);
        ~NetThread();
    };

}