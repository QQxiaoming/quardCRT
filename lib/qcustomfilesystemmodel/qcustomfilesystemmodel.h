#ifndef QCUSTOMFILESYSTEMMODEL_H
#define QCUSTOMFILESYSTEMMODEL_H

#include <QAbstractItemModel>

class QCustomFileSystemItem {
public:
    QCustomFileSystemItem(const QString &path, QCustomFileSystemItem *parent = 0);
    ~QCustomFileSystemItem();

    void appendChild(QCustomFileSystemItem *child);
    void removeChild(int row);

    QCustomFileSystemItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data() const;
    int row() const;
    QCustomFileSystemItem *parent();

private:
    QList<QCustomFileSystemItem*> m_childItems;
    QString m_path;
    QCustomFileSystemItem *m_parentItem;
};

class QCustomFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QCustomFileSystemModel(QObject *parent = 0);
    ~QCustomFileSystemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void fetchMore(const QModelIndex &parent);
    bool canFetchMore(const QModelIndex &parent) const;

    QModelIndex setRootPath(const QString &path);
    QModelIndex setRootPath(const char *path) { return setRootPath(QString::fromUtf8(path)); }
    QString rootPath();

    QString filePath(const QModelIndex &index);

    virtual QStringList pathEntryList(const QString &path) = 0;
    virtual bool isDir(const QString &path) = 0;
    virtual QString separator() = 0;
    virtual QVariant pathInfo(QString path, int type) const = 0;

private:
    QString m_rootPath;
    QCustomFileSystemItem *m_rootItem = nullptr;
};

class QNativeFileSystemModel : public QCustomFileSystemModel
{
    Q_OBJECT
public:
    explicit QNativeFileSystemModel(QObject *parent = 0);
    ~QNativeFileSystemModel();

    QStringList pathEntryList(const QString &path);
    bool isDir(const QString &path);
    QString separator();
    QVariant pathInfo(QString path, int type) const;
};


#endif // QCUSTOMFILESYSTEMMODEL_H
