#include "server.hpp"
#include "../shared_resources/include/network.hpp"
#include "../shared_resources/include/platform.hpp"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <string>
#include <sys/socket.h>

Server::Server(QObject *parent)
    : QObject(parent)
    , m_fileListModel(new FileListModel(this))
    , m_connected(false)
    , m_launched(false)
    , threadPool(std::make_shared<ThreadPool>(2))
{
}

Server::~Server()
{
    Network::close_socket(server_sockfd);
    Network::close_socket(communication_sockfd);
    Network::close_socket(transfer_sockfd);
}

// Initialize and set-up the server with GUI-user-input
void Server::launch(const QString Qipaddr, QString Qport)
{
    // convert ip from Qstring to C-style string and initialize
    this->ip = new char[std::strlen(Qipaddr.toUtf8().constData()) + 1];
    std::strcpy(const_cast<char *>(this->ip), Qipaddr.toUtf8().constData());
    // same conversion/initialization with port
    this->port = (Qport.toInt());
    threadPool->enqueue([this]() {
        if (!Network::create_server(server_sockfd, server_addr, this->ip, this->port)
            || !Network::bind_to_port(this->port, this->server_sockfd, this->server_addr)) {
            QMetaObject::invokeMethod(this, "setLaunched", Qt::QueuedConnection, Q_ARG(bool, false));
            return;
        }
        QMetaObject::invokeMethod(this, "setLaunched", Qt::QueuedConnection, Q_ARG(bool, true));
    });
    return;
}

// Listen and accept connection
void Server::accept()
{
    threadPool->enqueue([this]() {
        if (!Network::listen_client(server_sockfd)
            || !Network::accept_connection(server_sockfd, this->communication_sockfd)) {
            return;
        }
        QMetaObject::invokeMethod(this, "setIsConnected", Qt::QueuedConnection, Q_ARG(bool, true));
    });
    return;
}

// Receive file list
bool Server::receive_file_list()
{
    const int bufferSize = 1024;
    char buffer[bufferSize] = {0};
    QString data;

    while (true) {
        int bytes_received = Network::receive_data(communication_sockfd, buffer, bufferSize);
        if (bytes_received < 0) {
            std::cerr << "Error: Failed to receive data" << std::endl;
            return false;
        }
        if (bytes_received == 0) {
            std::cerr << "Error: Client failed to provide listed directory" << std::endl;
            return false;
        }

        data.append(QString::fromUtf8(buffer, bytes_received));

        // Check for termination condition
        if (data.contains('\0')) {
            break;
        }
    }

    data = data.trimmed();

    m_fileList = data.split('\n', Qt::SkipEmptyParts);

    // Update the model
    if (m_fileListModel) {
        bool success = QMetaObject::invokeMethod(m_fileListModel,
                                                 "setStringList",
                                                 Qt::QueuedConnection,
                                                 Q_ARG(QStringList, m_fileList));
        if (!success) {
            qWarning() << "invokeMethod failed";
        }
    } else {
        std::cerr << "Error: m_fileListModel is null" << std::endl;
    }
    QMetaObject::invokeMethod(this, "fileListReceived", Qt::QueuedConnection);
    return true;
}

// Request upload of a file
bool Server::request_upload(const QString &fname, const QString &Qfilename)
{
    QByteArray ba = fname.toUtf8();
    const char *filename = ba.data();
    if (!Network::send_data(communication_sockfd, filename, strlen(filename) + 1)) {
        std::cerr << "Error: Failed to provide client with a filename " << strerror(errno)
                  << std::endl;
        return false;
    };
    if (!change_port()) {
        std::cerr << "Error: Failed to change port ";
        return false;
    }
    std::cout << "Requested upload of a " << filename << " on a port "
              << ntohs(server_addr.sin_port) << std::endl;
    // New thread for upload
    threadPool->enqueue([this, Qfilename]() { receive_file(Qfilename); });
    return true;
}

// Change port
int Server::change_port() {
    if (transfer_sockfd > 0) {
        Network::close_socket(transfer_sockfd);
    }

    transfer_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);

    const int yes = 1;
    if (setsockopt(transfer_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        std::cerr << "Error setting SO_REUSEADDR" << std::strerror(errno) << std::endl;
        Network::close_socket(transfer_sockfd);
        return false;
    }

    if (port < 8400) {
        this->port = int(8400);
    } else {
        this->port += int(5);
    }

    Network::bind_to_port(port, transfer_sockfd, server_addr);

    Network::listen_client(transfer_sockfd);

    // Send new port to client
    if (!Network::send_data(communication_sockfd, &port, sizeof(port))) {
        return false;
        std::cerr << "Error: Failed to notify client about new port" << strerror(errno)
                  << std::endl;
    }
    transfer_sockfd = Network::accept_connection(transfer_sockfd, transfer_sockfd);
    std::cout << "Port changed to: " << ntohs(server_addr.sin_port) << std::endl;

    QMetaObject::invokeMethod(this, "portChanged", Qt::QueuedConnection);
    return transfer_sockfd;
}

// Receive the file
bool Server::receive_file(const QString &Qfilename)
{
        // Messages for client
        const char ACK[4] = "ACK";    // continue transfer
        const char CHPORT[4] = "CHP"; // change port

        // Get size of file
        size_t file_size;
        if (Network::receive_data(transfer_sockfd, &file_size, sizeof(file_size)) == -1) {
            std::cerr << "Error receiving file size" << std::endl;
            return false;
        }

        char *buffer = new char[file_size];
        std::string filename = Qfilename.mid(7).toStdString();

        // For dynamic port change and progress bar
        int percentage = 0;
        size_t total_bytes_received = 0;

        try {
            // Receive the file contents
            while (total_bytes_received < file_size) {
                // Calculate and log percentage
                percentage = static_cast<int>(static_cast<double>(total_bytes_received) * 100.0
                                              / file_size);
                // Update progress bar
                float progbar = static_cast<float>(percentage / 100.0);
                QMetaObject::invokeMethod(this,
                                          "setProgress",
                                          Qt::QueuedConnection,
                                          Q_ARG(float, float(progbar)));

                // Send ACK for start of the upload
                if (Network::send_data(this->communication_sockfd, ACK, sizeof(ACK)) == -1) {
                    std::cerr << "Error sending ACK" << std::endl;
                    throw std::runtime_error("Send ACK error");
                }

                // Determine chunk size
                size_t remaining_bytes = file_size - total_bytes_received;
                size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;

                // Receive chunk of file
                ssize_t bytes_received = Network::receive_data(this->transfer_sockfd,
                                                               buffer + total_bytes_received,
                                                               chunk_size);
                if (bytes_received == -1) {
                    std::cerr << "Error receiving data" << std::endl;
                    throw std::runtime_error("Receive data error");
                }

                total_bytes_received += bytes_received;

                if (bytes_received == 0) {
                    std::cout << "End of file transfer" << std::endl;
                    break;
                }

                // Check if it's time to change port
                int new_percentage = static_cast<int>(static_cast<double>(total_bytes_received)
                                                      / file_size * 100.0);
                if (new_percentage != percentage) {
                    percentage = new_percentage;
                    if ((percentage - 1) % 10 == 0) {
                        if (Network::send_data(communication_sockfd, CHPORT, sizeof(CHPORT)) == -1) {
                            std::cerr << "Error sending CHPORT" << std::endl;
                            throw std::runtime_error("Send CHPORT error");
                        }
                        this->transfer_sockfd = change_port();
                    }
                }
            }

            QMetaObject::invokeMethod(this, "setProgress", Qt::QueuedConnection, Q_ARG(float, 1));
            QMetaObject::invokeMethod(this, "transferSuccess", Qt::QueuedConnection);

        } catch (const std::exception &ex) {
            std::cerr << "Error receiving file: " << ex.what() << std::endl;
            Platform::cleanup_handler(buffer);
            return false;
        }

        if (sizeof(buffer) > 0) {
            Platform::write_file(filename, buffer, file_size);
        }

        Platform::cleanup_handler(buffer);

    return true;
}

// Props get-set-notify
int Server::getPort()
{
    return this->port;
}
QString Server::getIp() const
{
    return QString(this->ip);
}

bool Server::isLaunched() const
{
    return this->m_launched;
}
void Server::setLaunched(bool newLaunched)
{
    if (m_launched == newLaunched)
        return;
    this->m_launched = newLaunched;
    emit launched();
}

bool Server::isConnected() const
{
    return m_connected;
}
void Server::setIsConnected(bool newConnected)
{
    if (m_connected == newConnected)
        return;
    m_connected = newConnected;
    emit isConnectedChanged();
}

QStringList Server::getFileList() const
{
    return m_fileList;
}
FileListModel *Server::fileListModel() const
{
    return m_fileListModel;
}

float Server::progress() const
{
    return m_progress;
}
void Server::setProgress(float newProgress)
{
    if (m_progress == newProgress)
        return;
    m_progress = newProgress;
    emit progressChanged();
}

bool Server::success() const
{
    return m_transferSuccess;
}
void Server::setSuccess(bool newTransferSuccess)
{
    if (m_transferSuccess == newTransferSuccess)
        return;
    m_transferSuccess = newTransferSuccess;
    emit transferSuccess();
}
