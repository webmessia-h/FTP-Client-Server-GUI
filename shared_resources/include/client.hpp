// client
#pragma once
#include <QObject>
#include <QString>
#include "../serverGUI/threadpool.hpp"
#include "network.hpp"
#include "platform.hpp"
#include <qqml.h>
class Client : public QObject
{
    Q_OBJECT
    // Properties
    // ipaddr
    Q_PROPERTY(const QString ip READ get_ip NOTIFY ip_changed)
    // port
    Q_PROPERTY(int port READ getPort NOTIFY portChanged)
    // is file chosen by server
    Q_PROPERTY(bool fileChosen READ isChosen WRITE setIsChosen NOTIFY fileChosen)
    // is connected
    Q_PROPERTY(bool connected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
    // progress-bar property
    Q_PROPERTY(int progress READ progress WRITE setProgress NOTIFY progressChanged)
    // all good
    //Q_PROPERTY(bool transferSuccess READ success NOTIFY transferSuccess)

    QML_ELEMENT

public slots:

    void connect(const QString ipaddr, QString port);
    void send_file_list(const QString &qPath);
    void handle_request();
    void setProgress(int newProgress);
    void setIsChosen(bool newFileChosen);

signals:
    void portChanged();
    void ip_changed();
    void connected();
    void isConnectedChanged();
    void folder_selected();
    void fileChosen();
    void progressChanged(int progress);

public:
    explicit Client(QObject *parent = nullptr);

    ~Client();

    int getPort();
    QString get_ip() const;

    bool isConnected() const;
    void setIsConnected(bool newConnected);

    void set_base_directory(const std::string &path);

    int get_new_port();

    void send_file(const file_info &FILE);

    const char *ip;
    int port, client_sockfd, transfer_sockfd;
    struct sockaddr_in server_addr;

    int progress() const;

    bool isChosen() const;

private:
    std::shared_ptr<ThreadPool> threadPool;
    bool m_connected = false;
    std::string base_directory;
    bool m_fileChosen = false;
    int m_progress = 0;
};
