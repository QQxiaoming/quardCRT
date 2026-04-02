/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 */

#include "sessionprotocol.h"
#include <QMessageBox>
#include <QSerialPortInfo>

#include "sessionswindow.h"
#include "qextserialenumerator.h"

namespace sessionprotocol {
static SessionProtocolRegistry::ProtocolSpec serialProtocolSpec()
{
    return {
        "serial",
        "Serial",
        {
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "portName", "portName"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "baudRate", "baudRate"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "dataBits", "dataBits"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "parity", "parity"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "stopBits", "stopBits"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "flowControl", "flowControl"},
            {SessionProtocolRegistry::ProtocolMetaField::ProtocolMeta, "xEnable", "xEnable"}
        }
    };
}

class SerialProtocol final : public SessionProtocolBase {
public:
    struct StartArgs {
        QString portName;
        uint32_t baudRate = 0;
        int dataBits = 0;
        int parity = 0;
        int stopBits = 0;
        bool flowControl = false;
        bool xEnable = false;
    };

    SessionsWindow::SessionType type() const override { return SessionsWindow::Serial; }
    SessionsWindow::SessionCategory category() const override { return SessionsWindow::ConsoleSession; }
    void initialize(SessionsWindow *session) override {
        serialPort = new QSerialPort(session);
        session->realtimespeed_timer = new QTimer(session);

        QObject::connect(serialPort, &QSerialPort::readyRead, session, [=](){
            session->forwardReceivedData(serialPort->readAll(), true);
        });
        session->setupModemProxyForward([=]() {
            return session->state == SessionsWindow::Connected && serialPort->isOpen();
        }, [=](const QByteArray &data) {
            serialPort->write(data.data(), data.size());
        });
        session->setupTerminalSendForward([=]() {
            return session->state == SessionsWindow::Connected && serialPort->isOpen();
        }, [=](const char *data, int size) {
            serialPort->write(data, size);
        }, true);
        QObject::connect(serialPort, &QSerialPort::errorOccurred, session, [=](QSerialPort::SerialPortError serialPortError){
            if(serialPort->error() == QSerialPort::NoError) return;
            if(session->state == SessionsWindow::Error || session->state == SessionsWindow::Disconnected) return;
            session->reportSessionError(session->tr("Serial Error"), session->tr("Serial error:\n%0\n%1.").arg(serialPort->portName()).arg(serialPort->errorString()));
            serialPort->close();
            Q_UNUSED(serialPortError);
        });
        serialMonitor = new QextSerialEnumerator();
        serialMonitor->setUpNotifications();
    }
    void cloneTo(SessionsWindow *target,
                 const QVariantMap &commonMeta,
                 const QVariantMap &protocolMeta,
                 const QString &profile) override {
        Q_UNUSED(profile);
        Q_UNUSED(commonMeta);
        target->startSession(QVariantMap(), protocolMeta);
    }
    void disconnect(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(serialPort && serialPort->isOpen()) serialPort->close();
    }
    void fillStateInfo(SessionsWindow *session, SessionsWindow::StateInfo &info) override {
        info.serial.tx_total = session->tx_total;
        info.serial.rx_total = session->rx_total;
        info.serial.tx_speed = session->tx_speed;
        info.serial.rx_speed = session->rx_speed;
    }
    bool hasChildProcess(SessionsWindow *session) override {
        Q_UNUSED(session);
        return false;
    }
    void refreshTermSize(SessionsWindow *session) override {
        Q_UNUSED(session);
    }
    void cleanup(SessionsWindow *session) override {
        Q_UNUSED(session);
        if(serialPort) {
            if(serialPort->isOpen()) {
                serialPort->clear();
                serialPort->waitForBytesWritten(1000);
                serialPort->close();
            }
            delete serialPort;
            serialPort = nullptr;
        }
        if(serialMonitor) {
            delete serialMonitor;
            serialMonitor = nullptr;
        }
    }
    int startSerialSession(SessionsWindow *session, const QString &portName, uint32_t baudRate,
                           int dataBits, int parity, int stopBits, bool flowControl, bool xEnable) {
        if(!serialPort || !serialMonitor) {
            return -1;
        }
        portNameValue = portName;
        baudRateValue = baudRate;
        dataBitsValue = dataBits;
        parityValue = parity;
        stopBitsValue = stopBits;
        flowControlValue = flowControl;
        xEnableValue = xEnable;
        serialPort->setPortName(portName);
        serialPort->setBaudRate(baudRate);
        serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
        switch(parity)
        {
        case 0:
        default:
            serialPort->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialPort->setParity(QSerialPort::OddParity);
            break;
        case 2:
            serialPort->setParity(QSerialPort::EvenParity);
            break;
        }
        serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
        serialPort->setFlowControl(flowControl ? QSerialPort::HardwareControl : QSerialPort::NoFlowControl);
        if(!serialPort->open(QIODevice::ReadWrite)) {
            session->state = SessionsWindow::Error;
            emit session->stateChanged(session->state);
        } else {
            serialPort->setBreakEnabled(xEnable);
            session->updateConnectionState(true);
#if defined(Q_OS_WIN)
            QString monitorPortName = serialPort->portName();
#else
            QSerialPortInfo sinfo(*serialPort);
            QString monitorPortName = sinfo.systemLocation();
#endif
            QObject::connect(serialMonitor, &QextSerialEnumerator::deviceRemoved, session,
                    [&, monitorPortName](const QextPortInfo &info) {
                if(monitorPortName == info.portName) {
                    if(serialPort->isOpen()) {
                        serialPort->close();
                        QMessageBox::warning(session->messageParentWidget, session->tr("Serial Error"),
                                             session->getName() + "\n" + session->tr("Serial port %1 has been removed.").arg(info.portName));
                        session->state = SessionsWindow::Error;
                        emit session->stateChanged(session->state);
                    }
                }
            });
        }
        return 0;
    }
    QVariantMap exportMeta() const override {
        return {
            {"portName", portNameValue},
            {"baudRate", static_cast<qulonglong>(baudRateValue)},
            {"dataBits", dataBitsValue},
            {"parity", parityValue},
            {"stopBits", stopBitsValue},
            {"flowControl", flowControlValue},
            {"xEnable", xEnableValue}
        };
    }
    void importMeta(const QVariantMap &meta) override {
        portNameValue = meta.value("portName").toString();
        baudRateValue = static_cast<uint32_t>(meta.value("baudRate").toULongLong());
        dataBitsValue = meta.value("dataBits").toInt();
        parityValue = meta.value("parity").toInt();
        stopBitsValue = meta.value("stopBits").toInt();
        flowControlValue = meta.value("flowControl").toBool();
        xEnableValue = meta.value("xEnable").toBool();
    }
    int startSession(SessionsWindow *session,
                     const QVariantMap &commonMeta,
                     const QVariantMap &protocolMeta) override {
        Q_UNUSED(commonMeta);
        const StartArgs args = parseStartArgs(protocolMeta);
        return startSerialSession(session,
                                  args.portName,
                                  args.baudRate,
                                  args.dataBits,
                                  args.parity,
                                  args.stopBits,
                                  args.flowControl,
                                  args.xEnable);
    }

private:
    static StartArgs parseStartArgs(const QVariantMap &protocolMeta) {
        StartArgs args;
        args.portName = protocolMeta.value("portName").toString();
        args.baudRate = protocolMeta.value("baudRate").toUInt();
        args.dataBits = protocolMeta.value("dataBits").toInt();
        args.parity = protocolMeta.value("parity").toInt();
        args.stopBits = protocolMeta.value("stopBits").toInt();
        args.flowControl = protocolMeta.value("flowControl").toBool();
        args.xEnable = protocolMeta.value("xEnable").toBool();
        return args;
    }

    QSerialPort *serialPort = nullptr;
    QextSerialEnumerator *serialMonitor = nullptr;
    QString portNameValue;
    uint32_t baudRateValue = 0;
    int dataBitsValue = 0;
    int parityValue = 0;
    int stopBitsValue = 0;
    bool flowControlValue = false;
    bool xEnableValue = false;
};

SessionProtocolRegistrar kSerialProtocolRegistrar(
    SessionsWindow::Serial,
    []() { return std::make_unique<SerialProtocol>(); },
    serialProtocolSpec());
}
