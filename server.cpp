#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>

#define PORT "7777"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_handler(int s) {
    static_cast<void>(s);
    int saved_err = errno;
    while(waitpid(-1, NULL, WNOHANG)>0);
    errno = saved_err;
}

int main() {
    int sockfd, new_fd, status, err, yes = 1;
    struct addrinfo hints, *servinfo, *curr;
    struct sockaddr_storage their_addr;
    struct sigaction sa;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        std::cerr<<"gai error: "<<gai_strerror(status)<<'\n';
        exit(1);
    }

    for(curr = servinfo; curr != nullptr; curr = curr->ai_next) {
        if((sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1) {
            err = errno;
            std::cerr<<"server: socket: "<<strerror(err)<<'\n';
            errno = err;
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            std::cerr<<"setsockopt: "<<strerror(errno);
            close(sockfd);
            exit(1);
        } 
        if(bind(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
            err = errno;
            close(sockfd);
            std::cerr<<"bind: "<<strerror(err)<<'\n';
            errno = err;
            continue;
        }
        break;
    }
    err = errno;
    if(curr == nullptr) {
        std::cerr<<strerror(err)<<'\n';
        freeaddrinfo(servinfo);
        exit(1);
    }
    freeaddrinfo(servinfo);

    if(listen(sockfd, BACKLOG) == -1) {
        std::cerr<<"listen: "<<strerror(errno)<<'\n';
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, nullptr) == -1) {
        std::cerr<<"sigaction: "<<strerror(errno)<<'\n';
        exit(1);
    }

    std::cout<<"Waiting for connections...\n";

    while(1) {
        socklen_t addr_size = sizeof(their_addr);
        if((new_fd = accept(sockfd, (sockaddr*)&their_addr, &addr_size)) == -1) {
            std::cerr<<"server:new socket: "<<strerror(errno)<<'\n';
            continue;
        }
        std::cout<<"connection established\n";
        pid_t pid = fork();
        if(pid == -1) {
            std::cerr<<"fork: "<<strerror(errno)<<'\n';
            close(new_fd);
            continue;
        } 
        else if(pid == 0) {
            close(sockfd);
            if(send(new_fd, "Hello, client\n", 14, 0) == -1) {
                std::cerr<<"send: "<<strerror(errno)<<'\n';
            } 
            int bytes_rec;
            char msg[MAXDATASIZE+1];
            while((bytes_rec = recv(new_fd, msg, MAXDATASIZE, 0))>0) {
                msg[bytes_rec] = '\0';
                if(send(new_fd, msg, bytes_rec, 0) == -1) {
                    std::cerr<<"send: "<<strerror(errno)<<'\n';
                }
            }
            std::cout<<"connection ended\n";
            close(new_fd);
            if(bytes_rec == -1) {
                std::cerr<<"recv: "<<strerror(errno)<<'\n';
            }
            exit(0);
        }
        close(new_fd); 
    }
    return 0;
}