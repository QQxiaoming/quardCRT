#ifndef QCUSTOMFILESYSTEMMODEL_H
#define QCUSTOMFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QDateTime>

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
    void setSize(uint64_t size) { m_size = size; }
    uint64_t size() const { return m_size; }
    void setLastModified(QDateTime lastModified) { m_lastModified = lastModified; }
    QDateTime lastModified() const { return m_lastModified; }
    void setIsDir(bool isDir) { m_isDir = isDir; }
    bool isDir() const { return m_isDir; }

private:
    QList<QCustomFileSystemItem*> m_childItems;
    QString m_path;
    uint64_t m_size;
    bool m_isDir;
    QDateTime m_lastModified;
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

    virtual QString separator() const = 0;
    virtual QStringList pathEntryList(const QString &path) = 0;
    virtual void pathInfo(QString path, bool &isDir, uint64_t &size, QDateTime &lastModified) = 0;

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

    QString separator() const override;
    QStringList pathEntryList(const QString &path) override;
    void pathInfo(QString path, bool &isDir, uint64_t &size, QDateTime &lastModified) override;
};


#endif // QCUSTOMFILESYSTEMMODEL_H
