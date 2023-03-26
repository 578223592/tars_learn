#include "tc_epoll_server.h"

uint64_t H64(uint64_t x)
{
    return ((uint64_t)x) << 32;
}

tars::NetThread::NetThread(/* args */)
{
    	//建立用于通知和关闭的socket
    int iSocketType = SOCK_STREAM;
    int iDomain = AF_INET;

    m_shutdown_sock = socket(iDomain, iSocketType, 0);
    m_notify_sock = socket(iDomain, iSocketType, 0);
	
    if(m_shutdown_sock < 0)
    {
        std::cout<<"_shutdown_sock  invalid"<<std::endl;
    }

    if(m_notify_sock < 0)
    {
        std::cout<<"_notify_sock invalid"<<std::endl;
    }

	_response.response="";
	_response.uid = 0;
}

tars::NetThread::~NetThread()
{
}
void tars::NetThread::parseAddr(const std::string &sAddr, struct in_addr &stSinAddr)
{
    // 将点分十进制转为二进制整数
    int iRet = inet_pton(AF_INET, sAddr.c_str(), &stSinAddr);
    if (iRet < 0)
    {
        std::cout << "parseAddr iRet error" << std::endl;
    }
    else if (iRet == 0)
    {
        hostent stHostent;
        struct hostent *pstHostent;
        char buf[2048] = "\0";
        int iError;

        gethostbyname_r(sAddr.c_str(), &stHostent, buf, sizeof(buf), &pstHostent, &iError);

        if (pstHostent == NULL)
        {
            std::cout << "gethostbyname_r error! :" << std::endl;
        }
        else
        {
            stSinAddr = *(struct in_addr *)pstHostent->h_addr;
        }
    }
}
int tars::NetThread::bind(std::string &ip, int &port)
{
    // 建立server端接收请求用的socket
    // socket-->bind-->listen
    if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cerr << "socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
    int _optval = 1;
    if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&_optval, sizeof(_optval)) == -1)
    {
        std::cerr << "set socket refuse fail" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    sockaddr_in bindAddr;
    bindAddr.sin_family = AF_INET;
    // bindAddr.sin_addr.s_addr = INADDR_ANY;
    parseAddr(ip, bindAddr.sin_addr); // todo：为什么要解析地址，服务器不是应该可以接收任意地址吗
    bindAddr.sin_port = htons(port);
    if (::bind(m_sock, (const sockaddr *)&bindAddr, sizeof(bindAddr)))
    {
        std::cerr << "bind error" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "server has bind fd is :" << m_sock << std::endl;

    int iConnBackLog = 1024;
    if (::listen(m_sock, iConnBackLog) < 0)
    {
        std::cerr << "listen error" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 设置长连接
    // 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
    _optval = 1;
    if (setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const void *)&_optval, sizeof(_optval)))
    {
        std::cerr << "set SO_KEEPALIVE refuse fail" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 关闭tcp的naga合并发送，提高效率
    _optval = 1;
    if (setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const void *)&_optval, sizeof(_optval)))
    {
        std::cerr << "[TC_Socket::setTcpNoDelay] error" << std::endl;
        exit(EXIT_FAILURE);
    }
    linger stLinger;
    stLinger.l_onoff = 1;  // 在close socket调用后, 但是还有数据没发送完毕的时候容许逗留
    stLinger.l_linger = 5; // 容许逗留的时间为0秒

    if (setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (const void *)&stLinger, sizeof(linger)) == -1)
    {
        std::cout << "[TC_Socket::setNoCloseWait] error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞

    int val = 0;

    if ((val = fcntl(m_sock, F_GETFL)) < 0)
    {
        std::cerr << "get fcntl faild" << std::endl;
        exit(EXIT_FAILURE);
    }
    bool blocking = false;
    val |= O_NONBLOCK;
    if ((val = fcntl(m_sock, F_SETFL, val)) < 0)
    {
        std::cerr << "set fcntl faild" << std::endl;
        exit(EXIT_FAILURE);
    }

    return m_sock;
}
/*
创建tc epoller
目前iIndex没有任何意义
*/
void tars::NetThread::createEpoll(uint32_t iIndex)
{
    m_epoller.create(10240);
    // 现在还不是很懂为什么要携带这些数据
    m_epoller.add(m_shutdown_sock, H64(ET_CLOSE), EPOLLIN);
    m_epoller.add(m_notify_sock, H64(ET_NOTIFY), EPOLLIN);
    // 创建socket的文件描述符加上data：ET_LISTEN
    m_epoller.add(m_sock, H64(ET_LISTEN) | m_sock, EPOLLIN);

    int _total = 200000;

    for (uint32_t i = 1; i < _total; i++)
    {
        m_free.push_back(i);
        m_free_size++;
    }
    std::cout << "epoll create successful" << std::endl;
}

// 新的连接到来
//  新连接到来需要完成的事情：读取相关信息，注册epoll事件
// todo :目前传入参数fd还没有用到
bool tars::NetThread::accept(int fd)
{
    sockaddr_in addrIn;
    socklen_t addrInLen = sizeof(addrIn);
    int newFd = ::accept(m_sock, (sockaddr *)&addrIn, &addrInLen);
    if (newFd == -1)
    {
        std::cerr << "::accept fail!!!" << std::endl;
        return false;
    }

    std::string ip;
    uint16_t port = 0;
    char sAddr[INET_ADDRSTRLEN] = "\0";

    inet_ntop(AF_INET, (const void *)&addrIn.sin_addr.s_addr, sAddr, sizeof(sAddr));

    ip = sAddr;
    port = ntohs(addrIn.sin_port);
    // show
    std::cout << "accept function:" << std::endl;
    std::cout << "  ip:" << ip << std::endl;
    std::cout << "  port:" << port << std::endl;
    // 对新来的连接也要进行一些设置
    //  只是不用设置端口复用等，其他都是一样的设置
    // 使用 长连接 关闭合并发送，使用 非阻塞，允许逗留
    // 设置长连接
    // 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
    int _optval = 1;
    if (setsockopt(newFd, SOL_SOCKET, SO_KEEPALIVE, (const void *)&_optval, sizeof(_optval)))
    {
        std::cerr << "set SO_KEEPALIVE refuse fail" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 关闭tcp的naga合并发送，提高效率
    _optval = 1;
    if (setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY, (const void *)&_optval, sizeof(_optval)))
    {
        std::cerr << "[TC_Socket::setTcpNoDelay] error" << std::endl;
        exit(EXIT_FAILURE);
    }
    linger stLinger;
    stLinger.l_onoff = 1;  // 在close socket调用后, 但是还有数据没发送完毕的时候容许逗留
    stLinger.l_linger = 5; // 容许逗留的时间为0秒

    if (setsockopt(newFd, SOL_SOCKET, SO_LINGER, (const void *)&stLinger, sizeof(linger)) == -1)
    {
        std::cout << "[TC_Socket::setNoCloseWait] error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞

    int val = 0;

    if ((val = fcntl(newFd, F_GETFL)) < 0)
    {
        std::cerr << "get fcntl faild" << std::endl;
        exit(EXIT_FAILURE);
    }
    bool blocking = false;
    val |= O_NONBLOCK;
    if ((val = fcntl(newFd, F_SETFL, val)) < 0)
    {
        std::cerr << "set fcntl faild" << std::endl;
        exit(EXIT_FAILURE);
    }
    //2023-3-25：每次的uid从m_free列表推出，严格保证不重复
    uint32_t uid = m_free.front();

    m_free.pop_front();

    --m_free_size;

    // 保存connectid
    m_listen_connect_id[uid] = newFd;

    // 注册到epoll模型
    //！！！重点：推测这里因为可以携带的数据是uint64，因此化成了高低32位，高32位负责记录事件类型notify，enet等
    // 低32位负责记录发生的uid，这里只有uid，那么意思就是默认事件类型为0
    // 又因为结构体里面专门写了enet为0，因此这里就相当于默认事件为enet了，这也就是为什么没有注册enet事件，但是却
    // 可以监听到的原因
    // m_epoller.add(newFd, uid, EPOLLIN | EPOLLOUT);
    m_epoller.add(newFd, H64(ET_NET) | uid, EPOLLIN | EPOLLOUT);

    return true;
}

void tars::NetThread::run()
{
    std::cout << "start to run ..." << std::endl;
    std::cout << "--------------------" << std::endl;


    // 自己写的服务器是根据event里面的事件来判断，而这里同意使用携带的数据来判断

    while (true)
    {
        int eventNum = m_epoller.wait(-1);
        for (int i = 0; i < eventNum; i++)
        {
            const epoll_event &event = m_epoller.get(i);
            uint32_t eventData = event.data.u64 >> 32;
            // 这个移位操作是与传入data的操作是镜像对称的
            switch (eventData)
            {
                // 新连接
            case ET_LISTEN:
                std::cout << "new connect!!" << std::endl;
                // different：源代码是do while 来循环接收连接
                if (event.events && EPOLLIN)
                {
                    bool ret = accept(0);
                }
                break;
            case ET_CLOSE:
                std::cout << "ET_CLOSE  but now do nothing" << std::endl;

                break;
            case ET_NOTIFY:
                std::cout << "ET_NOTIFY" << std::endl;
                processPipe();
                break;
            case ET_NET:
            // todo：断开连接的时候会进入这里，问题是从来没有注册过ET_NET，为什么会进入这里呢？？？
                std::cout << "ET_NET" << std::endl;
                processNet(event);
                break;
            default:
                std::cerr << "data error!" << std::endl;
                assert(false);
            }
        }
    }
}

void tars::NetThread::processNet(const epoll_event &ev)
{
    // todo:根据data.u32取出uid，那么什么时候装载进去的呢？？？
    uint32_t uid = ev.data.u32;
    int fd = m_listen_connect_id[uid];

    std::cout << "processNet uid is " << uid << " fd is " << fd << std::endl;
    //对方断开连接
    if (ev.events & EPOLLERR || ev.events & EPOLLHUP)
    {
        std::cout << "should delet connection" << std::endl;
        return;
    }

    if (ev.events & EPOLLIN)
    {
        while (true)
        {
            char buffer[32 * 1024];
            int iBytesReceived = 0;

            iBytesReceived = ::read(fd, (void *)buffer, sizeof(buffer));

            std::cout << "server recieve " << iBytesReceived << " bytes buffer is " << buffer << std::endl;

            if (iBytesReceived < 0)
            {
                //EAGAIN（非阻塞I/O操作返回，因为当前没有输入数据可用
                if (errno == EAGAIN)
                {
                    break;
                }
                else
                {
                    std::cout << "client close" << std::endl;
                    return;
                }
            }
            //返回值为0，表示已经读到文件末尾（End of File, EOF）
            else if (iBytesReceived == 0)
            {
                std::cout << "1 client close" << std::endl;
                return;
            }

            m_recvbuffer.append(buffer, iBytesReceived);

            _response.response = "this is service response string";
            _response.uid = uid;

            m_epoller.mod(m_notify_sock, H64(ET_NOTIFY), EPOLLOUT);
        }
    }

    if (ev.events & EPOLLOUT)
    {
        // 这里是处理上次未发送完的数据
        std::cout << "need to send data" << std::endl;
    }
}

void tars::NetThread::processPipe()
{
    uint32_t uid = _response.uid;

    int fd = m_listen_connect_id[uid];

    std::cout << "processPipe uid is " << uid << " fd is " << fd << std::endl;

    int bytes = ::send(fd, _response.response.c_str(), _response.response.size(), 0);

    if(bytes == -1){
        std::cout<<strerror(errno)<<std::endl;
    }
    std::cout << "send byte is " << bytes << "   ";

    std::cout << "send response is :" << _response.response << std::endl;
}
