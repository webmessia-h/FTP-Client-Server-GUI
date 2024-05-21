#pragma once

#include <QAbstractListModel>
#include <QStringList>

class FileListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { StringRole = Qt::UserRole + 1 };

    explicit FileListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
public slots:
    void setStringList(const QStringList &stringList);

private:
    QStringList m_stringList;
};
