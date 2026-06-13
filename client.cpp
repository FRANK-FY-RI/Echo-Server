#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PORT "7777"
#define MAXDATASIZE 100

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr<<"Usage client hostname\n";
        return 1;
    }
    int sockfd, status, err;
    struct addrinfo hints, *servinfo, *curr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        std::cerr<<"gai error: "<<gai_strerror(status)<<'\n';
        return 1;
    }
    for(curr = servinfo; curr != nullptr; curr = curr->ai_next) {
        if((sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1) {
            err = errno;
            std::cerr<<"server: socket: "<<strerror(errno)<<'\n';
            errno = err;
            continue;
        }
        if(connect(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
            err = errno;
            std::cerr<<"server: connect: "<<strerror(errno)<<'\n';
            errno = err;
            close(sockfd);
            continue;
        }
        break;
    }
    if(curr == nullptr) {
        err = errno;
        std::cerr<<"connection failed: "<<strerror(errno)<<'\n';
        freeaddrinfo(servinfo);
        return err;
    }
    freeaddrinfo(servinfo);
    std::cout<<"connection successful\n";

    auto rec = [](int sockfd) {
        int bytes_rec;
        char msg[MAXDATASIZE+1];
        std::cout<<"server: ";
        if((bytes_rec = recv(sockfd, msg, MAXDATASIZE, 0)) == -1) {
            std::cerr<<"recv: "<<strerror(errno)<<'\n';
            return;
        } 
        msg[bytes_rec] = '\0';
        std::cout<<msg<<'\n';
    };

    rec(sockfd); 
    // std::cout<<"reached getline loop\n";
    std::string s;
    while(std::getline(std::cin, s)) {
        // std::cout<<s<<'\n';
        if(s == "exit") break;
        if(send(sockfd, s.c_str(), s.size(), 0) == -1) {
            std::cerr<<"send: "<<strerror(errno)<<'\n';
        } 
        
        rec(sockfd);
    }
    close(sockfd);

    return 0;
}