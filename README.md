# 学习tars源码

> https://cloud.tencent.com/developer/article/1381523?areaSource=&traceId=


此版代码对应文章中的 1.1部分：https://github.com/578223592/tars_learn/commit/53590ea975a2d9e9dfffa3a58061557beefeaab5

运行状态： 使用 ET_NET事件 来输入输出数据 ， 使用 ET_LISTEN事件 来accept新连接

> 图片无法快捷上传，运行状态看原帖吧

2023-3-28：已经修复，但是还存在一些疑惑点
，运行后显示如下：
```bash
swx@swx-virtual-machine1:~/tars_learn$ ./run.sh 
server has bind fd is :5
epoll create successful
start to run ...
--------------------
ET_CLOSE  but now do nothing
ET_NOTIFY
processPipe uid is 0 fd is 0
Socket operation on non-socket
send byte is -1   send response is :
new connect!!
accept function:
  ip:127.0.0.1
  port:40116
ET_NET
processNet uid is 1 fd is 7
server recieve 14 bytes buffer is this is client
server recieve -1 bytes buffer is this is client
need to send data
```
只在tc_epoll_server.cpp中修改了 void tars::NetThread::processNet(const epoll_event &ev) 中的  m_epoller.mod(m_listen_sock.getfd(), H64(ET_NOTIFY), EPOLLOUT); 修改成了  m_epoller.mod(m_notify_sock.getfd(), H64(ET_NOTIFY), EPOLLOUT);即可。为什么呢？

现在已经明白了修改之后的含义，当准备发送的时候（读取完毕之后）注册m_notify_sock上面的可写事件，由于是et触发模式，因此会触发一次ET_NOTIFY信息，因此会发送一次信息，
> todo：修改后的逻辑搞清楚了，但是错误的 m_epoller.mod(m_listen_sock.getfd(), H64(ET_NOTIFY), EPOLLOUT); 也注册了事件（错误注册会导致携带的信息变成ET_NOTIFY，且只有可写事件了）为什么不会触发至少一次的返回事件呢？

---

在processnet函数中的代码与学习的代码还有些不一样，好像是添加注册事件那块，仔细看一下
ans：已经研究明白，关键部分就是event.data是一个union，变量相互影响，由于注册事件的时候就添加了文件描述符，因此使用u32就可以取出来了。

1.2版本代码修复完成
----

