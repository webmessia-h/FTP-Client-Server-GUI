// server
#pragma once
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include "../shared_resources/include/network.hpp"
#include "../shared_resources/include/platform.hpp"
#include "../shared_resources/include/threadpool.hpp"
#include "fileListModel.hpp"
#include <qqml.h>
class Server : public QObject
{
    Q_OBJECT
    //Properties
    // ipaddr
    Q_PROPERTY(const QString ip READ getIp NOTIFY ipChanged)
    // port
    Q_PROPERTY(int port READ getPort NOTIFY portChanged)
    // is launched
    Q_PROPERTY(bool launched READ isLaunched NOTIFY launched)
    // is connected
    Q_PROPERTY(bool connected READ isConnected NOTIFY isConnectedChanged)
    // file list received
    Q_PROPERTY(QStringList fileList READ getFileList NOTIFY fileListReceived)
    // custom file-list model as data isn't present at instantiation
    Q_PROPERTY(FileListModel *fileListModel READ fileListModel CONSTANT)
    // progress-bar property
    Q_PROPERTY(float progress READ progress WRITE setProgress NOTIFY progressChanged)
    // all good
    Q_PROPERTY(bool transferSuccess READ success WRITE setSuccess NOTIFY transferSuccess)

    QML_ELEMENT

public slots:
    // Launch the server
    void launch(const QString ipaddr, QString port);
    void setLaunched(bool newLaunched);

    // Accept connection
    void accept();
    void setIsConnected(bool newConnected);

    // Receive and print listed folder
    bool receive_file_list();
    FileListModel *fileListModel() const;

    // Request desired file from the client
    bool request_upload(const QString &fname, const QString &Qfilename);

    // Receive file
    bool receive_file(const QString &Qfilename);
    void setProgress(float newProgress);
    void setSuccess(bool newTransferSuccess);

signals:

    void portChanged();
    void ipChanged();

    void launched();

    void isConnectedChanged();

    void fileListReceived();

    void fileChosen();

    void progressChanged();

    void transferSuccess();

public:
    Server(const char *ip, const int port);
    ~Server();
    explicit Server(QObject *parent = nullptr);

    // Props getter-setter's
    int getPort();
    QString getIp() const;

    bool isLaunched() const;

    bool isConnected() const;

    QStringList getFileList() const;

    /*bool isChosen() const;
    void setIsChosen(bool newFileChosen);*/

    float progress() const;

    bool success() const;

    // Handle port change
    int change_port();

    int server_sockfd, port, communication_sockfd, transfer_sockfd;
    struct sockaddr_in server_addr;
    const char *ip;

private:
    std::shared_ptr<ThreadPool> threadPool;
    bool m_connected = false;
    bool m_launched = false;
    QStringList m_fileList;
    FileListModel *m_fileListModel;
    bool m_file_chosen = false;
    float m_progress = 0.0;
    bool m_transferSuccess = false;
};
