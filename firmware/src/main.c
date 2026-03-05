#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "project_config.h"
#include "protocol_v1.h"
#include "protocol_engine.h"

static size_t build_cmd_packet(uint16_t seq,
                               uint16_t cmd_id,
                               const void *payload,
                               uint16_t payload_len,
                               uint8_t *out,
                               size_t out_capacity) {
    const size_t total_len = sizeof(protocol_v1_header_t) + sizeof(protocol_v1_cmd_envelope_t) + payload_len;
    protocol_v1_header_t *hdr;
    protocol_v1_cmd_envelope_t *cmd;

    if (out_capacity < total_len) {
        return 0;
    }

    hdr = (protocol_v1_header_t *)out;
    cmd = (protocol_v1_cmd_envelope_t *)(out + sizeof(protocol_v1_header_t));

    hdr->magic = PROTOCOL_V1_MAGIC;
    hdr->version = PROTOCOL_V1_VERSION;
    hdr->kind = PROTO_KIND_CMD;
    hdr->seq = seq;
    hdr->len = (uint16_t)(sizeof(protocol_v1_cmd_envelope_t) + payload_len);
    hdr->crc16 = 0;

    cmd->cmd_id = cmd_id;
    cmd->reserved = 0;

    if (payload_len > 0 && payload != NULL) {
        memcpy(cmd->payload, payload, payload_len);
    }

    hdr->crc16 = protocol_v1_crc16_ccitt_false(out, (uint32_t)total_len);
    return total_len;
}

static void print_rsp_summary(const uint8_t *packet, size_t packet_len) {
    const protocol_v1_header_t *hdr;
    const protocol_v1_rsp_envelope_t *rsp;

    if (packet_len < sizeof(protocol_v1_header_t) + sizeof(protocol_v1_rsp_envelope_t)) {
        printf("rsp too short\n");
        return;
    }

    hdr = (const protocol_v1_header_t *)packet;
    rsp = (const protocol_v1_rsp_envelope_t *)(packet + sizeof(protocol_v1_header_t));

    printf("rsp seq=%u cmd=0x%04X status=%u payload=%u\n",
           hdr->seq,
           rsp->cmd_id,
           rsp->status,
           (unsigned)(hdr->len - sizeof(protocol_v1_rsp_envelope_t)));
}

int main(void) {
    uint8_t in_packet[256];
    uint8_t out_packet[256];
    size_t in_len;
    size_t out_len;
    protocol_engine_state_t engine;

    stdio_init_all();
    protocol_engine_init(&engine);

    const uint8_t probe[] = {0x44, 0x53, 0x01, 0x01};
    const uint16_t probe_crc = protocol_v1_crc16_ccitt_false(probe, sizeof(probe));

    printf("rppico-scope firmware bootstrap\n");
    printf("usb vid:pid = 0x%04X:0x%04X\n", RPPICO_USB_VID, RPPICO_USB_PID);
    printf("usb strings: %s / %s\n", RPPICO_USB_MANUFACTURER, RPPICO_USB_PRODUCT);
    printf("pins ch1=%u ch2=%u fg_pwm_adc_pin=%u\n",
           RPPICO_SCOPE_ADC_CH1,
           RPPICO_SCOPE_ADC_CH2,
           RPPICO_FG_PWM_ADC_CHANNEL);
    printf("safe adc max aggregate = %u sps\n", (unsigned)RPPICO_SAFE_ADC_MAX_AGGREGATE_SPS);
    printf("protocol v%u magic=0x%04X crc_probe=0x%04X\n",
           PROTOCOL_V1_VERSION,
           PROTOCOL_V1_MAGIC,
           probe_crc);

    in_len = build_cmd_packet(1, PROTO_CMD_PING, probe, sizeof(probe), in_packet, sizeof(in_packet));
    out_len = protocol_engine_handle_packet(&engine, in_packet, in_len, out_packet, sizeof(out_packet));
    print_rsp_summary(out_packet, out_len);

    in_len = build_cmd_packet(2, PROTO_CMD_GET_CAPS, NULL, 0, in_packet, sizeof(in_packet));
    out_len = protocol_engine_handle_packet(&engine, in_packet, in_len, out_packet, sizeof(out_packet));
    print_rsp_summary(out_packet, out_len);

    in_len = build_cmd_packet(3, PROTO_CMD_GET_SCOPE_CFG, NULL, 0, in_packet, sizeof(in_packet));
    out_len = protocol_engine_handle_packet(&engine, in_packet, in_len, out_packet, sizeof(out_packet));
    print_rsp_summary(out_packet, out_len);

    while (true) {
        sleep_ms(1000);
    }
}
