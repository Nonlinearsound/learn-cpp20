#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "fd.h"

// RAII helper class for encapsulating the file descriptor for the socket
// we need this as in the constructor of the Socket class when throwing an exception,
// we would need to manually close the socket, which is not a clean way.
// class Fd {
//     int fd;
// public:
//     explicit Fd(int f = -1) : fd(f) {}
//     ~Fd() { if (fd >= 0) ::close(fd); }
//
//     int get() const { return fd; }
//     void reset(int newfd = -1) {
//         if (fd >= 0) ::close(fd);
//         fd = newfd;
//     }
// };


class Socket {
    Fd sockfd;
    std::vector<char> v_data;
    int timeout;

public:
    Socket() {
        timeout = 5;
        v_data = std::vector<char>();
        int fd;
        if ((fd = ::socket(AF_INET, SOCK_STREAM, 0) < 0) ){
            throw std::runtime_error("Could not create socket");
        }
        sockfd.reset(fd);

        // set timeout for thr socket to 5 seconds for now
        timeval tv{};
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        if ( ::setsockopt(sockfd.get(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ) {
            perror("setsockopt");
            throw std::runtime_error("Could not set socket timeout");
        }

        std::cout << "socket created with fd=" << sockfd.get() << std::endl;
    }

    void connectTo(const std::string& ip, int port) {
        v_data.clear();
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        if (::inet_pton(AF_INET,ip.c_str(),&serverAddr.sin_addr) <= 0 ) {
            throw std::runtime_error("Invalid ip address");
        }

        if (::connect(  sockfd.get(),
                        reinterpret_cast<const sockaddr*>(&serverAddr),
                        sizeof(serverAddr)) < 0 ) {
            throw std::runtime_error("Could not connect to the server");
        }

        std::cout << "Connected to " << ip << ":" << port << "\n";
    }

    std::vector<char> sendData(const std::string& data) {
        ssize_t sent = ::send( sockfd.get(),data.c_str(),data.size(),0);
        if ( sent <= 0 ) {
            throw std::runtime_error("Send failed");
        }
        std::cout << "Sent: " << sent << std::endl;

        char buffer[4096];

        ssize_t received;
        while ( (received = ::recv(sockfd.get(),buffer,sizeof(buffer),0 )) > 0  ) {
            v_data.insert(v_data.end(),buffer, buffer+received);
        }

        if (received < 0) {
            throw std::runtime_error("Recv failed");
        }

        // 4. Return a copy of the data
        return v_data;
    }

    ~Socket() = default;

};

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    std::cout << "Hello and welcome to " << std::endl;
    try {
        auto *socket = new Socket();
        socket->connectTo("23.192.228.80",80);
        std::vector<char> data = socket->sendData("GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n");
        std::cout << "Received data: " << std::string(data.begin(),data.end()) << std::endl;
    } catch (std::exception& e) {
        std::cout << "An exception occurred: " << e.what() << std::endl;
    }

    return 0;
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}