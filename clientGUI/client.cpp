#include "client.hpp"
#include "../shared_resources/include/network.hpp"
#include "../shared_resources/include/platform.hpp"
#include <qobjectdefs.h>
#include <string>
#include <sys/socket.h>
Client::Client(QObject *parent)
    : QObject(parent), m_connected(false), m_fileChosen(false),
      threadPool(std::make_shared<ThreadPool>(1)) {}

Client::~Client() {
  Network::close_socket(this->client_sockfd);
  Network::close_socket(this->transfer_sockfd);
}

void Client::connect(const QString Qipaddr, QString Qport) {
  // convert ip from Qstring to C-style string and initialize
  this->ip = new char[std::strlen(Qipaddr.toUtf8().constData()) + 1];
  std::strcpy(const_cast<char *>(this->ip), Qipaddr.toUtf8().constData());
  // same conversion/initialization with port
  this->port = (Qport.toInt());
  // create client communication socket
  if (!Network::create_client(this->client_sockfd))
    return;
  // connect client socket to server
  if (!Network::connect_to_server(client_sockfd, server_addr, this->ip,
                                  this->port))
    return;
  // i think self-explanatory
  setIsConnected(true);
  emit ip_changed();
  emit portChanged();
  emit connected();
  return;
}

// Send listed directory
void Client::send_file_list(const QString &qPath) {
  // as QFolderDialog passes folder as URI 'file:///' we must clip this prefix
  // MOVE TO HELPERS
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
    // apend pairs to a string
    file_info += file + "\n";
  }

  emit folder_selected();
  set_base_directory(path);
  // cast to C-style string and send
  Network::send_data(client_sockfd, file_info.c_str(), file_info.size());
}

// store path to listed dir
void Client::set_base_directory(const std::string &path) {
  this->base_directory = path;
}

// get and set new port
int Client::get_new_port() {
  if (this->transfer_sockfd > 0) {
    Network::close_socket(this->transfer_sockfd);
  }
  this->transfer_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);
  int Newport = 0;
  Network::receive_data(client_sockfd, &Newport, sizeof(port));
  this->port = Newport;
  if (!Network::connect_to_server(this->transfer_sockfd, this->server_addr, ip,
                                  port)) {
    return false;
  }
  QMetaObject::invokeMethod(this, "portChanged", Qt::QueuedConnection);
  return transfer_sockfd;
}

// send file with dynamic port change
void Client::send_file(const std::string &filename,
                       const std::string &base_directory) {

  file_info FILE = Platform::read_file(filename, base_directory);
  // Send file size
  if (Network::send_data(this->transfer_sockfd, &FILE.file_size,
                         sizeof(FILE.file_size)) == -1) {
    std::cerr << "Error sending file size" << std::endl;
    return;
  }

  // Send file
  size_t total_bytes_sent = 0;
  int percentage = 0;
  try {

    while (total_bytes_sent < FILE.file_size) {
      // Calculate percentage
      percentage = static_cast<int>(static_cast<double>(total_bytes_sent) *
                                    100.0 / FILE.file_size);
      // Update progress bar
      float progbar = static_cast<float>(percentage / 100.0);
      QMetaObject::invokeMethod(this, "setProgress", Qt::QueuedConnection,
                                Q_ARG(float, float(progbar)));

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
        // Check if the file stream is good before reading
        if (!FILE.file_stream.good()) {
          std::cerr << "Error: File stream is not good before reading. Total "
                       "bytes sent: "
                    << total_bytes_sent << std::endl;
          throw std::runtime_error("File stream error");
        }

        // Send size calculation (currently 1Kb)
        size_t remaining_bytes = FILE.file_size - total_bytes_sent;
        size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;
        // Read 1Kb into buffer
        std::vector<char> buffer(chunk_size);
        FILE.file_stream.read(buffer.data(), chunk_size);

        // send 1Kb chunk of file
        ssize_t bytes_sent = Network::send_data(this->transfer_sockfd,
                                                buffer.data(), chunk_size);
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
    QMetaObject::invokeMethod(this, "setProgress", Qt::QueuedConnection,
                              Q_ARG(float, 1.0));
    QMetaObject::invokeMethod(this, "transferSuccess", Qt::QueuedConnection);
  }
  // Exception handle
  catch (const std::exception &ex) {
    std::cerr << "Error sending file: " << ex.what() << std::endl;
  }
  // Explicitly close the file stream
  if (FILE.file_stream.is_open()) {
    FILE.file_stream.close();
    std::cout << "File stream closed successfully." << std::endl;
  }
  threadPool->stopped();
}

// handle upload request
void Client::handle_request() { // New thread for sending
  // get filename
  char filename[256] = {0};
  Network::receive_data(client_sockfd, filename, sizeof(filename));
  // recieved filename and ready to upload
  QMetaObject::invokeMethod(this, "setIsChosen", Qt::QueuedConnection,
                            Q_ARG(bool, true));
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

  // New thread for sending
  threadPool->enqueue(
      [this, filename_str]() { send_file(filename_str, base_directory); });

  return;
}

bool Client::isConnected() const { return m_connected; }
void Client::setIsConnected(bool newConnected) {
  if (m_connected == newConnected)
    return;
  m_connected = newConnected;
  emit isConnectedChanged();
}

int Client::getPort() { return this->port; }
QString Client::get_ip() const { return QString(this->ip); }

bool Client::isChosen() const { return m_fileChosen; }
void Client::setIsChosen(bool newFileChosen) {
  if (m_fileChosen == newFileChosen)
    return;
  m_fileChosen = newFileChosen;
  emit fileChosen();
}

float Client::progress() const { return m_progress; }
void Client::setProgress(float newProgress) {
  if (m_progress == newProgress)
    return;
  m_progress = newProgress;
  emit progressChanged(m_progress);
}

bool Client::success() const { return m_transferSuccess; }
void Client::setSuccess(bool newTransferSuccess) {
  if (m_transferSuccess == newTransferSuccess)
    return;
  m_transferSuccess = newTransferSuccess;
  emit transferSuccess();
}
