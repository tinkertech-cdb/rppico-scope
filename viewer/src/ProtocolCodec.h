#ifndef PROTOCOLCODEC_H
#define PROTOCOLCODEC_H

#include <QByteArray>

#include "protocol_v1.h"

class ProtocolCodec {
public:
    static QByteArray makeCommand(quint16 seq, quint16 cmdId, const QByteArray &payload);
    static bool parseResponse(const QByteArray &packet, protocol_v1_header_t &header, protocol_v1_rsp_envelope_t &rsp, QByteArray &payload);
    static quint16 crc16(const uint8_t *data, uint32_t len);
};

#endif
