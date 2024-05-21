#include "client.hpp"
#include "network.hpp"
#include "platform.hpp"
#include <sys/socket.h>
Client::Client(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_fileChosen(false)
    , threadPool(std::make_shared<ThreadPool>(2))
{}

void Client::connect(const QString Qipaddr, QString Qport)
{
    // convert ip from Qstring to C-style string and initialize
    this->ip = new char[std::strlen(Qipaddr.toUtf8().constData()) + 1];
    std::strcpy(const_cast<char *>(this->ip), Qipaddr.toUtf8().constData());
    // same conversion/initialization with port
    this->port = (Qport.toInt());
    // create client communication socket
    if (!Network::create_client(this->client_sockfd))
        return;
    //connect client socket to server
    if (!Network::connect_to_server(client_sockfd, server_addr, this->ip, this->port))
        return;
    // i think self-explanatory
    setIsConnected(true);
    emit ip_changed();
    emit portChanged();
    emit connected();
    return;
}

// Send listed directory
void Client::send_file_list(const QString &qPath)
{
    // as QFolderDialog passes folder as URI 'file:///' we must clip this prefix MOVE TO HELPERS
    std::string path;
    path = qPath.mid(7).toStdString();

    std::string file_info = {0};
    std::vector<std::string> files =
        // Function to make a pair of filename and its size
        Platform::list_directory(path);
    if (files.empty()) {
        return;
    }

    for (const auto &file : files) {
        //apend pairs to a string
        file_info += file + "\n";
    }

    emit folder_selected();
    set_base_directory(path);
    // cast to C-style string and send
    Network::send_data(client_sockfd, file_info.c_str(), file_info.size());
}

// store path to listed dir
void Client::set_base_directory(const std::string &path)
{
    this->base_directory = path;
}

// get and set new port
int Client::get_new_port()
{
    if (this->transfer_sockfd > 0) {
        Network::close_socket(this->transfer_sockfd);
    }
    this->transfer_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);
    int Newport = 0;
    Network::receive_data(client_sockfd, &Newport, sizeof(port));
    this->port = Newport;
    if (!Network::connect_to_server(this->transfer_sockfd, this->server_addr, ip, port)) {
        return false;
    }
    QMetaObject::invokeMethod(this, "portChanged", Qt::QueuedConnection);
    return transfer_sockfd;
}

// send file with dynamic port change
void Client::send_file(const file_info &FILE)
{
    // Send file size
    if (Network::send_data(this->transfer_sockfd, &FILE.file_size, sizeof(FILE.file_size)) == -1) {
        std::cerr << "Error sending file size" << std::endl;
        return;
    }

    // Send file
    size_t total_bytes_sent = 0;
    try {
        while (total_bytes_sent < FILE.file_size) {
            int percentage = static_cast<int>(static_cast<double>(total_bytes_sent) * 100.0
                                              / FILE.file_size);
            // Update progress bar
            QMetaObject::invokeMethod(this,
                                      "setProgress",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, percentage));
            // Receive ACK or CHPROT from server
            char msg[4] = {0};
            if (Network::receive_data(this->client_sockfd, msg, sizeof(msg)) == -1) {
                std::cerr << "Error receiving message" << std::endl;
                throw std::runtime_error("Receive error");
            }

            // Change port
            if (msg[0] == 'C') {
                this->transfer_sockfd = get_new_port();
                continue;
            }
            // Keep sending
            else if (msg[0] == 'A') {
                size_t remaining_bytes = FILE.file_size - total_bytes_sent;
                size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;
                ssize_t bytes_sent = Network::send_data(this->transfer_sockfd,
                                                        FILE.buffer + total_bytes_sent,
                                                        chunk_size);

                if (bytes_sent == -1) {
                    std::cerr << "Error sending data"
                              << ", bytes sent:" << total_bytes_sent << std::endl;
                    throw std::runtime_error("Send error");
                }
                total_bytes_sent += bytes_sent;
            } else {
                std::cerr << "Unexpected message from server: " << msg << std::endl;
                throw std::runtime_error("Unexpected server message");
            }
        }
        // Set progress 100 as transferred
        QMetaObject::invokeMethod(this, "setProgress", Qt::QueuedConnection, Q_ARG(int, 100));
    }
    // Exception handle
    catch (const std::exception &ex) {
        std::cerr << "Error sending file: " << ex.what() << std::endl;
        // Handle error and clean up
        Platform::cleanup_handler(FILE.buffer);
        return;
    } catch (const std::runtime_error &err) {
        std::cerr << "Error sending file: " << err.what() << std::endl;
        // Handle error and clean up
        Platform::cleanup_handler(FILE.buffer);
        return;
    }

    // Ensure cleanup is called on success
    Platform::cleanup_handler(FILE.buffer);
}

// handle upload request
void Client::handle_request()
{
    threadPool->enqueue([this]() {
        // get filename
        char filename[256] = {0};
        Network::receive_data(client_sockfd, filename, sizeof(filename));
        // recieved filename and ready to upload
        QMetaObject::invokeMethod(this, "setIsChosen", Qt::QueuedConnection, Q_ARG(bool, true));
        // create new socket for file transfer
        Network::create_client(transfer_sockfd);
        // get new port
        Client::get_new_port();

        // Remove size and unit info
        std::string filename_str(filename);
        size_t pos = filename_str.find(" - ");
        if (pos != std::string::npos) {
            filename_str = filename_str.substr(0, pos);
        }

        // write file into buffer
        file_info FILE = Platform::read_file(filename_str, base_directory);
        if (FILE.buffer == nullptr) {
            std::cerr << "Error: Failed to deliver file to client " << strerror(errno) << std::endl;
            return;
        }
        std::cout << "Handle request: Calling send_file()" << std::endl; // Log message
        send_file(FILE);
        Platform::cleanup_handler(FILE.buffer);
    });
}

bool Client::isConnected() const
{
    return m_connected;
}
void Client::setIsConnected(bool newConnected)
{
    if (m_connected == newConnected)
        return;
    m_connected = newConnected;
    emit isConnectedChanged();
}

int Client::getPort()
{
    return this->port;
}
QString Client::get_ip() const
{
    return QString(this->ip);
}

bool Client::isChosen() const
{
    return m_fileChosen;
}
void Client::setIsChosen(bool newFileChosen)
{
    if (m_fileChosen == newFileChosen)
        return;
    m_fileChosen = newFileChosen;
    emit fileChosen();
}

int Client::progress() const
{
    return m_progress;
}

void Client::setProgress(int newProgress)
{
    if (m_progress == newProgress)
        return;
    m_progress = newProgress;
    emit progressChanged(m_progress);
}

Client::~Client()
{
    Network::close_socket(this->client_sockfd);
    Network::close_socket(this->transfer_sockfd);
}
