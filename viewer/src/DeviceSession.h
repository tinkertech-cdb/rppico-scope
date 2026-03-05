#ifndef DEVICESESSION_H
#define DEVICESESSION_H

#include <QByteArray>
#include <QString>

class DeviceSession {
public:
    DeviceSession();

    bool connectDevice(QString &statusText);
    void disconnectDevice(QString &statusText);

    bool isConnected() const;

    bool ping(QString &resultText);
    bool getCaps(QString &resultText);
    bool startStream(QString &resultText);
    bool stopStream(QString &resultText);

private:
    bool m_connected;
    quint16 m_seq;

    QByteArray issueMockCommand(quint16 cmdId, const QByteArray &payload);
};

#endif
