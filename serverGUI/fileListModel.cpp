#include "fileListModel.hpp"

FileListModel::FileListModel(QObject *parent) : QAbstractListModel(parent) {}

int FileListModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent);
  return m_stringList.count();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role == StringRole)
    return m_stringList.at(index.row());

  return QVariant();
}

QHash<int, QByteArray> FileListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[StringRole] = "string";
  return roles;
}

void FileListModel::setStringList(const QStringList &stringList) {
  beginResetModel();
  m_stringList = stringList;
  endResetModel();
}
