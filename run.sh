g++ -o tar-demo tc_epoll_server.cpp main.cpp tc_epoller.cpp tc_socket.cpp -g
g++ -o client client.cpp -g

./tar-demo