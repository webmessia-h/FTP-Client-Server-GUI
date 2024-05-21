// network
#pragma once
#ifdef _WIN32
#include <winsock2.h> // Windows
#define SOCKET int
#else

#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include <string.h>
#include <sys/types.h> // For socket types
class Network
{
public:
    static int create_socket(int domain, int type, int protocol);
    // Establish connection between server and client
    // Initialize server socket
    static bool create_server(int &server_sockfd,
                              struct sockaddr_in &server_addr,
                              const char *ip,
                              int port);
    // Bind server's socket
    static bool bind_to_port(int port, int &server_sockfd, struct sockaddr_in &server_addr);
    // Listen for incoming connections
    static bool listen_client(int &server_sockfd);
    // Accept pending connection request
    static int accept_connection(int &server_sockfd, int &communication_sockfd);
    // Initialize client's socket
    static int create_client(int &client_sockfd);
    // Make connection request
    static bool connect_to_server(int &client_sockfd,
                                  struct sockaddr_in &server_addr,
                                  const char *ip,
                                  int port);
    // Send data with some logging
    static size_t send_data(int socket_fd, const void *data, size_t size);
    // Receive data with some logging
    static size_t receive_data(int socket_fd, void *buffer, size_t size);
    // Close socket with some logging
    static void close_socket(int socket_fd);
    //
};
