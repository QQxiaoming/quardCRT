#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>
#include <QObject>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QLocale>
#include <QList>

#define PLUGIN_API_VERSION 1

class PluginInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~PluginInterface() {}
    virtual int init(QMap<QString, QString> params, QWidget *parent) = 0;
    virtual QString name() = 0;
    virtual QString version() = 0;
    virtual QMenu *mainMenu() = 0;
    virtual QAction *mainAction() = 0;
    virtual QMenu *terminalContextMenu(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual QList<QAction *> terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu) = 0;
    virtual void setLanguage(const QLocale &language,QApplication *app) = 0;
    virtual void retranslateUi() = 0;

signals:
    void requestTelnetConnect(QString host, int port, int type);
    void requestSerialConnect(QString portName, uint32_t baudRate, int dataBits, int parity, int stopBits, bool flowControl, bool xEnable);
    void requestLocalShellConnect(QString command, QString workingDirectory);
    void requestRawSocketConnect(QString host, int port);
    void requestNamePipeConnect(QString namePipe);
    void requestSSH2Connect(QString host, QString user, QString password, int port);
    void requestVNCConnect(QString host, QString password, int port);
    void sendCommand(QString cmd);
    void writeSettings(QString path, QString key, QVariant value);
    void readSettings(QString path, QString key, QVariant &value);
};

#define PluginInterface_iid "org.quardCRT.PluginInterface"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
