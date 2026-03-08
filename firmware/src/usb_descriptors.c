#include "tusb.h"

#include <stddef.h>
#include <string.h>

#include "protocol_v1.h"
#include "protocol_engine.h"

#define USBD_VID 0xCAFE
#define USBD_PID 0x4010
#define USBD_MANUFACTURER "OpenScope Community"
#define USBD_PRODUCT "XIAO RP2350 Scope+FG"

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_VENDOR,
    ITF_NUM_MAX
};

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

#define EPNUM_VENDOR_OUT   0x03
#define EPNUM_VENDOR_IN    0x83

#define TUSB_DESC_CONFIG_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN)

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_MAX, 0, TUSB_DESC_CONFIG_LEN, 0, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, 0, EPNUM_VENDOR_OUT, EPNUM_VENDOR_IN, 64),
};

static const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

static const uint8_t *desc_device_arr[] = { (const uint8_t *)&desc_device };

void tud_mount_cb(void) {
    printf("USB mounted\n");
}

void tud_umount_cb(void) {
    printf("USB unmounted\n");
}

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t desc_str[32];
    
    if (index == 0) {
        desc_str[0] = (uint16_t)((1 << 8) | 0x0409);
        return desc_str;
    }
    
    const char *str = NULL;
    if (index == 1) str = USBD_MANUFACTURER;
    else if (index == 2) str = USBD_PRODUCT;
    
    if (str) {
        size_t len = strlen(str);
        if (len > 31) len = 31;
        desc_str[0] = (uint16_t)(((len + 1) << 8) | TUSB_DESC_STRING);
        for (size_t i = 0; i < len; i++) {
            desc_str[i + 1] = str[i];
        }
        desc_str[len + 1] = 0;
        return desc_str;
    }
    
    return NULL;
}

