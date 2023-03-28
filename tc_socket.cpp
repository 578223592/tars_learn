#include "tc_socket.h"
#include <assert.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <unistd.h>
namespace tars
{
    static const int INVALID_SOCKET = -1;
}

tars::TC_Socket::TC_Socket() : m_sock(tars::INVALID_SOCKET), m_bOwner(true), m_iDomain(AF_INET)
{
}

tars::TC_Socket::~TC_Socket()
{
    if (m_bOwner)
    {
        close();
    }
}
void tars::TC_Socket::init(int fd, bool bOwner, int iDomain)
{
    if (m_bOwner)
    {
        close();
    }

    m_sock = fd;
    m_bOwner = bOwner;
    m_iDomain = iDomain;
}

void tars::TC_Socket::createSocket(int iSocketType, int iDomain)
{
    assert(iSocketType == SOCK_STREAM || iSocketType == SOCK_DGRAM);
    close();

    m_iDomain = iDomain;
    m_sock = socket(iDomain, iSocketType, 0);

    if (m_sock < 0)
    {
        m_sock = INVALID_SOCKET;
        std::cout << "create socket error" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int tars::TC_Socket::getfd() const
{
    return m_sock;
}

int tars::TC_Socket::accept(tars::TC_Socket &tcSock, struct sockaddr *pstSockAddr, socklen_t &iSockLen)
{
    assert(tcSock.m_sock == INVALID_SOCKET);

    int ifd;

    while ((ifd = ::accept(m_sock, pstSockAddr, &iSockLen)) < 0 && errno == EINTR) ;

    tcSock.m_sock = ifd;
    tcSock.m_iDomain = m_iDomain;

    return tcSock.m_sock;
}

void tars::TC_Socket::bind(const std::string &sServerAddr, int port)
{
    assert(m_iDomain == AF_INET);

    struct sockaddr_in bindAddr;

    bzero(&bindAddr, sizeof(bindAddr));

    bindAddr.sin_family = m_iDomain;
    bindAddr.sin_port = htons(port);

    if (sServerAddr == "")
    {
        bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        parseAddr(sServerAddr, bindAddr.sin_addr);
    }

    try
    {
        bind((struct sockaddr *)(&bindAddr), sizeof(bindAddr));
    }
    catch (...)
    {
        std::cout << "[TC_Socket::bind] bind error" << std::endl;
    }
}

void tars::TC_Socket::parseAddr(const std::string &sAddr, struct in_addr &stSinAddr)
{
    int iRet = ::inet_pton(AF_INET, sAddr.c_str(), &stSinAddr);
    if (iRet < 0)
    {
        std::cout << "[TC_Socket::parseAddr] inet_pton error" << std::endl;
        ;
    }
    else if (iRet == 0)
    {
        struct hostent stHostent;
        struct hostent *pstHostent;
        char buf[2048] = "\0";
        int iError;

        gethostbyname_r(sAddr.c_str(), &stHostent, buf, sizeof(buf), &pstHostent, &iError);

        if (pstHostent == NULL)
        {
            std::cout << "[TC_Socket::parseAddr] gethostbyname_r error! :" << std::endl;
        }
        else
        {
            stSinAddr = *(struct in_addr *)pstHostent->h_addr;
        }
    }
}

void tars::TC_Socket::bind(struct sockaddr *pstBindAddr, socklen_t iAddrLen)
{
    // 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间
    int iReuseAddr = 1;

    // 设置
    setSockOpt(SO_REUSEADDR, (const void *)&iReuseAddr, sizeof(int), SOL_SOCKET);

    if (::bind(m_sock, pstBindAddr, iAddrLen) < 0)
    {
        std::cout << "[TC_Socket::bind] bind error" << std::endl;
    }
}

void tars::TC_Socket::close()
{
    if (m_sock != INVALID_SOCKET)
    {
        ::close(m_sock);
        m_sock = INVALID_SOCKET;
    }
}

int tars::TC_Socket::connectNoThrow(const std::string &sServerAddr, uint16_t port)
{
    assert(m_iDomain == AF_INET);

    if (sServerAddr == "")
    {
        std::cout << "[TC_Socket::connect] server address is empty error!" << std::endl;
    }

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));

    serverAddr.sin_family = m_iDomain;
    parseAddr(sServerAddr, serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    return connect((struct sockaddr *)(&serverAddr), sizeof(serverAddr));
}

void tars::TC_Socket::connect(const std::string &sServerAddr, uint16_t port)
{
    int ret = connectNoThrow(sServerAddr, port);

    if (ret < 0)
    {
        std::cout << "[TC_Socket::connect] connect error" << std::endl;
    }
}

int tars::TC_Socket::connect(struct sockaddr *pstServerAddr, socklen_t serverLen)
{
    return ::connect(m_sock, pstServerAddr, serverLen);
}

void tars::TC_Socket::listen(int iConnBackLog)
{
    if (::listen(m_sock, iConnBackLog) < 0)
    {
        std::cout << "[TC_Socket::listen] listen error" << std::endl;
    }
}

void tars::TC_Socket::setblock(bool bBlock)
{
    assert(m_sock != INVALID_SOCKET);

    setblock(m_sock, bBlock);
}

void tars::TC_Socket::setblock(int fd, bool bBlock)
{
    int val = 0;

    if ((val = fcntl(fd, F_GETFL, 0)) == -1)
    {
        std::cout << "[TC_Socket::setblock] fcntl [F_GETFL] error" << std::endl;
    }

    if (!bBlock)
    {
        val |= O_NONBLOCK;
    }
    else
    {
        val &= ~O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, val) == -1)
    {
        std::cout << "[TC_Socket::setblock] fcntl [F_SETFL] error" << std::endl;
    }
}

int tars::TC_Socket::setSockOpt(int opt, const void *pvOptVal, socklen_t optLen, int level)
{
    return setsockopt(m_sock, level, opt, pvOptVal, optLen);
}

void tars::TC_Socket::setKeepAlive()
{
    int flag = 1;
    if (setSockOpt(SO_KEEPALIVE, (char *)&flag, int(sizeof(int)), SOL_SOCKET) == -1)
    {
        std::cout << "[TC_Socket::setKeepAlive] error" << std::endl;
    }
}

void tars::TC_Socket::setTcpNoDelay()
{
    int flag = 1;

    if (setSockOpt(TCP_NODELAY, (char *)&flag, int(sizeof(int)), IPPROTO_TCP) == -1)
    {
        std::cout << "[TC_Socket::setTcpNoDelay] error" << std::endl;
    }
}

void tars::TC_Socket::setNoCloseWait()
{
    linger stLinger;
    stLinger.l_onoff = 1;  // 在close socket调用后, 但是还有数据没发送完毕的时候容许逗留
    stLinger.l_linger = 0; // 容许逗留的时间为0秒

    if (setSockOpt(SO_LINGER, (const void *)&stLinger, sizeof(linger), SOL_SOCKET) == -1)
    {
        std::cout << "[TC_Socket::setNoCloseWait] error" << std::endl;
    }
}

void tars::TC_Socket::setCloseWaitDefault()
{
    linger stLinger;
    stLinger.l_onoff = 0;
    stLinger.l_linger = 0;

    if (setSockOpt(SO_LINGER, (const void *)&stLinger, sizeof(linger), SOL_SOCKET) == -1)
    {
        std::cout << "[TC_Socket::setCloseWaitDefault] error" << std::endl;
    }
}
