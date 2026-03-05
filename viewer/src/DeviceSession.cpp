#include "DeviceSession.h"

#include <cstring>

#include "ProtocolCodec.h"

DeviceSession::DeviceSession() : m_connected(false), m_seq(1) {}

bool DeviceSession::connectDevice(QString &statusText) {
    m_connected = true;
    statusText = "Status: Connected (mock transport)";
    return true;
}

void DeviceSession::disconnectDevice(QString &statusText) {
    m_connected = false;
    statusText = "Status: Disconnected";
}

bool DeviceSession::isConnected() const {
    return m_connected;
}

QByteArray DeviceSession::issueMockCommand(quint16 cmdId, const QByteArray &payload) {
    const QByteArray cmdPacket = ProtocolCodec::makeCommand(m_seq++, cmdId, payload);
    protocol_v1_header_t cmdHdr;
    std::memcpy(&cmdHdr, cmdPacket.constData(), sizeof(cmdHdr));

    QByteArray rspPayload;
    quint16 status = PROTO_STATUS_OK;
    if (cmdId == PROTO_CMD_GET_CAPS) {
        protocol_v1_device_caps_t caps{};
        caps.feature_bits = PROTO_FEAT_DUAL_CHANNEL_SCOPE |
                            PROTO_FEAT_FUNCTION_GENERATOR |
                            PROTO_FEAT_TRIGGER_SINGLE |
                            PROTO_FEAT_STREAM_DROP_COUNTER;
        caps.max_channels = 2;
        caps.max_adc_ksps_agg = 500;
        caps.min_sample_rate_hz = 100;
        caps.max_sample_rate_hz = 500000;
        caps.max_stream_payload = 2048;
        caps.fg_freq_min_millihz = 1000;
        caps.fg_freq_max_millihz = 200000000;
        caps.fg_amp_min_mvpp = 0;
        caps.fg_amp_max_mvpp = 3300;
        caps.fg_offset_min_mv = -1650;
        caps.fg_offset_max_mv = 1650;
        rspPayload = QByteArray(reinterpret_cast<const char *>(&caps), sizeof(caps));
    } else if (cmdId == PROTO_CMD_PING) {
        rspPayload = payload;
    } else if (cmdId == PROTO_CMD_STREAM_START || cmdId == PROTO_CMD_STREAM_STOP) {
        rspPayload.clear();
    } else {
        status = PROTO_ERR_BAD_CMD;
    }

    const quint16 rspLen = static_cast<quint16>(sizeof(protocol_v1_rsp_envelope_t) + rspPayload.size());
    QByteArray rspPacket(static_cast<int>(sizeof(protocol_v1_header_t) + rspLen), Qt::Uninitialized);

    auto *rspHdr = reinterpret_cast<protocol_v1_header_t *>(rspPacket.data());
    auto *rsp = reinterpret_cast<protocol_v1_rsp_envelope_t *>(rspPacket.data() + sizeof(protocol_v1_header_t));

    rspHdr->magic = PROTOCOL_V1_MAGIC;
    rspHdr->version = PROTOCOL_V1_VERSION;
    rspHdr->kind = PROTO_KIND_RSP;
    rspHdr->seq = cmdHdr.seq;
    rspHdr->len = rspLen;
    rspHdr->crc16 = 0;

    rsp->cmd_id = cmdId;
    rsp->status = status;
    if (!rspPayload.isEmpty()) {
        std::memcpy(rsp->payload, rspPayload.constData(), static_cast<size_t>(rspPayload.size()));
    }

    rspHdr->crc16 = ProtocolCodec::crc16(reinterpret_cast<const uint8_t *>(rspPacket.constData()),
                                         static_cast<uint32_t>(rspPacket.size()));

    return rspPacket;
}

bool DeviceSession::ping(QString &resultText) {
    if (!m_connected) {
        resultText = "Ping failed: not connected";
        return false;
    }

    const QByteArray probe("PING", 4);
    const QByteArray rspPacket = issueMockCommand(PROTO_CMD_PING, probe);

    protocol_v1_header_t hdr{};
    protocol_v1_rsp_envelope_t rsp{};
    QByteArray payload;
    if (!ProtocolCodec::parseResponse(rspPacket, hdr, rsp, payload)) {
        resultText = "Ping failed: malformed response";
        return false;
    }

    resultText = QString("Ping OK: seq=%1 payload=%2")
                     .arg(hdr.seq)
                     .arg(QString::fromLatin1(payload));
    return true;
}

bool DeviceSession::getCaps(QString &resultText) {
    if (!m_connected) {
        resultText = "GET_CAPS failed: not connected";
        return false;
    }

    const QByteArray rspPacket = issueMockCommand(PROTO_CMD_GET_CAPS, QByteArray());

    protocol_v1_header_t hdr{};
    protocol_v1_rsp_envelope_t rsp{};
    QByteArray payload;
    if (!ProtocolCodec::parseResponse(rspPacket, hdr, rsp, payload)) {
        resultText = "GET_CAPS failed: malformed response";
        return false;
    }

    if (rsp.status != PROTO_STATUS_OK || payload.size() != static_cast<int>(sizeof(protocol_v1_device_caps_t))) {
        resultText = "GET_CAPS failed: bad status or payload";
        return false;
    }

    protocol_v1_device_caps_t caps{};
    std::memcpy(&caps, payload.constData(), sizeof(caps));

    resultText = QString("Caps: channels=%1 maxAgg=%2kS/s streamPayload=%3")
                     .arg(caps.max_channels)
                     .arg(caps.max_adc_ksps_agg)
                     .arg(caps.max_stream_payload);
    return true;
}

bool DeviceSession::startStream(QString &resultText) {
    if (!m_connected) {
        resultText = "STREAM_START failed: not connected";
        return false;
    }

    const QByteArray rspPacket = issueMockCommand(PROTO_CMD_STREAM_START, QByteArray());
    protocol_v1_header_t hdr{};
    protocol_v1_rsp_envelope_t rsp{};
    QByteArray payload;

    if (!ProtocolCodec::parseResponse(rspPacket, hdr, rsp, payload) || rsp.status != PROTO_STATUS_OK) {
        resultText = "STREAM_START failed";
        return false;
    }

    resultText = QString("Streaming started (seq=%1)").arg(hdr.seq);
    return true;
}

bool DeviceSession::stopStream(QString &resultText) {
    if (!m_connected) {
        resultText = "STREAM_STOP failed: not connected";
        return false;
    }

    const QByteArray rspPacket = issueMockCommand(PROTO_CMD_STREAM_STOP, QByteArray());
    protocol_v1_header_t hdr{};
    protocol_v1_rsp_envelope_t rsp{};
    QByteArray payload;

    if (!ProtocolCodec::parseResponse(rspPacket, hdr, rsp, payload) || rsp.status != PROTO_STATUS_OK) {
        resultText = "STREAM_STOP failed";
        return false;
    }

    resultText = QString("Streaming stopped (seq=%1)").arg(hdr.seq);
    return true;
}
