#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    int sockfd = -1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("sock created");
        exit(-1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton("127.0.0.1", &server_addr.sin_addr) == 0) // 服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }

    server_addr.sin_port = htons(9877);
    socklen_t server_addr_length = sizeof(server_addr);
    // 向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if (connect(sockfd, (struct sockaddr *)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect!\n");
        exit(1);
    }

    printf("connect successfull\n");

    char recvBuf[1024] = {0};

    string request = "this is client";

    while (true)
    {
        request = "this is client";
        cout << "准备发送" << endl;
        int ret = write(sockfd, request.c_str(), request.size());
        if(ret == - 1){
            cerr << strerror(errno) << endl;
        }
        cout << "已经发送：" << request << " 成功 开始读取." << endl;
        ret = read(sockfd, recvBuf, sizeof(recvBuf));
        if(ret == - 1){
            cerr << strerror(errno) << endl;
        }
        fputs(recvBuf, stdout);
        printf("\n");
        memset(recvBuf, 0, sizeof(recvBuf));
        sleep(1);
    }

    close(sockfd);

    return 0;
}
