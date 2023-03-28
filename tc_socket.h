#pragma

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <sys/un.h>
#include <iostream>
namespace tars
{
    class TC_Socket
    {
    public:
        TC_Socket();
        virtual ~TC_Socket();

        /// @brief 初始化tc_socket
        /// @param fd 绑定的文件描述符 ？？但是怎么可能指定绑定哪个呢？还是先手动调用再来绑定？
        /// @param bOwner  
        /// @param iDomain 域
        void init(int fd, bool bOwner, int iDomain = AF_INET);

        /// @brief 关闭原有socket，创建新的socket
        /// @param iSocketType socket类型：SOCK_STREAM 或者 SOCK_DGRAM
        /// @param iDomain 域
        void createSocket(int iSocketType = SOCK_STREAM, int iDomain = AF_INET);

        int getfd() const ;

        void close();

        void setOwner(bool bOwner) { m_bOwner = bOwner; }

        int setSockOpt(int opt, const void *pvOptVal, socklen_t optLen, int level = SOL_SOCKET);

        /// @brief 创建新的连接，三个参数均为会修改的指针传递
        /// @param tcSock 
        /// @param pstSockAddr 
        /// @param iSockLen 
        /// @return 新绑定的fd
        int accept(TC_Socket &tcSock, struct sockaddr *pstSockAddr, socklen_t &iSockLen);

        /// @brief 绑定ip 和 port  ，并设置端口重用
        /// @param sServerAddr 
        /// @param port 
        void bind(const std::string &sServerAddr, int port);

        // void bind(const char *sPathName);

        

        void connect(const std::string &sServerAddr, uint16_t port);

        int connect(struct sockaddr *pstServerAddr, socklen_t serverLen);

        int connectNoThrow(const std::string &sServerAddr, uint16_t port);

        void listen(int connBackLog);

        void setKeepAlive();

        static void setblock(int fd, bool bBlock);

        void setblock(bool bBlock = false);

        void parseAddr(const std::string &sAddr, struct in_addr &stSinAddr);

        void setTcpNoDelay();

        void setNoCloseWait();

        void setCloseWaitDefault();

    protected:
        
        /// @brief socket对应的fd
        int m_sock;

        /// @brief 暂时未知
        bool m_bOwner;

        /// @brief 域
        int m_iDomain;
    private:
    void bind(struct sockaddr *pstBindAddr, socklen_t iAddrLen);
    };

} // namespace tars
