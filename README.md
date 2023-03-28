# 学习tars源码

> https://cloud.tencent.com/developer/article/1381523?areaSource=&traceId=


此版代码对应文章中的 1.1部分：https://github.com/578223592/tars_learn/commit/53590ea975a2d9e9dfffa3a58061557beefeaab5

运行状态： 使用 ET_NET事件 来输入输出数据 ， 使用 ET_LISTEN事件 来accept新连接

> 图片无法快捷上传，运行状态看原帖吧


目前还存在bug，待修复，运行后显示如下：
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
