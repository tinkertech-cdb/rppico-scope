#include "ProtocolCodec.h"

#include <cstring>

quint16 ProtocolCodec::crc16(const uint8_t *data, uint32_t len) {
    quint16 crc = 0xFFFFu;

    for (uint32_t i = 0; i < len; ++i) {
        crc ^= static_cast<quint16>(data[i]) << 8;
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000u) {
                crc = static_cast<quint16>((crc << 1) ^ 0x1021u);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

QByteArray ProtocolCodec::makeCommand(quint16 seq, quint16 cmdId, const QByteArray &payload) {
    const quint16 cmdLen = static_cast<quint16>(sizeof(protocol_v1_cmd_envelope_t) + payload.size());
    QByteArray packet(static_cast<int>(sizeof(protocol_v1_header_t) + cmdLen), Qt::Uninitialized);

    auto *hdr = reinterpret_cast<protocol_v1_header_t *>(packet.data());
    auto *cmd = reinterpret_cast<protocol_v1_cmd_envelope_t *>(packet.data() + sizeof(protocol_v1_header_t));

    hdr->magic = PROTOCOL_V1_MAGIC;
    hdr->version = PROTOCOL_V1_VERSION;
    hdr->kind = PROTO_KIND_CMD;
    hdr->seq = seq;
    hdr->len = cmdLen;
    hdr->crc16 = 0;

    cmd->cmd_id = cmdId;
    cmd->reserved = 0;
    if (!payload.isEmpty()) {
        std::memcpy(cmd->payload, payload.constData(), static_cast<size_t>(payload.size()));
    }

    hdr->crc16 = crc16(reinterpret_cast<const uint8_t *>(packet.constData()), static_cast<uint32_t>(packet.size()));
    return packet;
}

bool ProtocolCodec::parseResponse(const QByteArray &packet,
                                  protocol_v1_header_t &header,
                                  protocol_v1_rsp_envelope_t &rsp,
                                  QByteArray &payload) {
    if (packet.size() < static_cast<int>(sizeof(protocol_v1_header_t) + sizeof(protocol_v1_rsp_envelope_t))) {
        return false;
    }

    std::memcpy(&header, packet.constData(), sizeof(header));
    if (header.magic != PROTOCOL_V1_MAGIC || header.version != PROTOCOL_V1_VERSION || header.kind != PROTO_KIND_RSP) {
        return false;
    }

    if (packet.size() != static_cast<int>(sizeof(protocol_v1_header_t) + header.len)) {
        return false;
    }

    QByteArray crcInput = packet;
    auto *tmpHdr = reinterpret_cast<protocol_v1_header_t *>(crcInput.data());
    const quint16 receivedCrc = tmpHdr->crc16;
    tmpHdr->crc16 = 0;
    if (crc16(reinterpret_cast<const uint8_t *>(crcInput.constData()), static_cast<uint32_t>(crcInput.size())) != receivedCrc) {
        return false;
    }

    std::memcpy(&rsp,
                packet.constData() + sizeof(protocol_v1_header_t),
                sizeof(protocol_v1_rsp_envelope_t));

    const int payloadOffset = static_cast<int>(sizeof(protocol_v1_header_t) + sizeof(protocol_v1_rsp_envelope_t));
    const int payloadLen = header.len - static_cast<int>(sizeof(protocol_v1_rsp_envelope_t));
    payload = packet.mid(payloadOffset, payloadLen);
    return true;
}
