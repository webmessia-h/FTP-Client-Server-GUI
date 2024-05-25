// client
#pragma once
#include "../shared_resources/include/network.hpp"
#include "../shared_resources/include/platform.hpp"
#include "../shared_resources/include/threadpool.hpp"
#include <QObject>
#include <QString>
#include <qqml.h>
#include <string>
class Client : public QObject {
  Q_OBJECT
  // Properties
  // ipaddr
  Q_PROPERTY(const QString ip READ get_ip NOTIFY ip_changed)
  // port
  Q_PROPERTY(int port READ getPort NOTIFY portChanged)
  // is file chosen by server
  Q_PROPERTY(bool fileChosen READ isChosen WRITE setIsChosen NOTIFY fileChosen)
  // is connected
  Q_PROPERTY(bool connected READ isConnected WRITE setIsConnected NOTIFY
                 isConnectedChanged)
  // progress-bar property
  Q_PROPERTY(
      float progress READ progress WRITE setProgress NOTIFY progressChanged)
  // all good
  Q_PROPERTY(
      bool transferSuccess READ success WRITE setSuccess NOTIFY transferSuccess)

  QML_ELEMENT

public slots:

  void connect(const QString ipaddr, QString port);
  void send_file_list(const QString &qPath);

  void handle_request();
  void setIsChosen(bool newFileChosen);

  void setProgress(float newProgress);
  void setSuccess(bool newTransferSuccess);

signals:

  void portChanged();
  void ip_changed();
  void connected();
  void isConnectedChanged();

  void folder_selected();
  void fileChosen();

  void progressChanged(float progress);

  void transferSuccess();

public:
  explicit Client(QObject *parent = nullptr);

  ~Client();

  int getPort();
  QString get_ip() const;

  bool isConnected() const;
  void setIsConnected(bool newConnected);

  void set_base_directory(const std::string &path);

  bool isChosen() const;

  int get_new_port();

  float progress() const;

  bool success() const;

  void send_file(const std::string &filename,
                 const std::string &base_directory);

  const char *ip;
  int port, client_sockfd, transfer_sockfd;
  struct sockaddr_in server_addr;

private:
  std::shared_ptr<ThreadPool> threadPool;
  bool m_connected = false;
  std::string base_directory;
  bool m_fileChosen = false;
  float m_progress = 0.0;
  bool m_transferSuccess = false;
};
