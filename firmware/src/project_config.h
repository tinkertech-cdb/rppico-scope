#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#ifndef RPPICO_USB_VID
#define RPPICO_USB_VID 0xCAFE
#endif

#ifndef RPPICO_USB_PID
#define RPPICO_USB_PID 0x4010
#endif

#ifndef RPPICO_USB_MANUFACTURER
#define RPPICO_USB_MANUFACTURER "OpenScope Community"
#endif

#ifndef RPPICO_USB_PRODUCT
#define RPPICO_USB_PRODUCT "XIAO RP2350 Scope+FG"
#endif

#define RPPICO_SCOPE_ADC_CH1 0
#define RPPICO_SCOPE_ADC_CH2 1

/* ADC2 pin is used in GPIO mode for PWM function-generator output. */
#define RPPICO_FG_PWM_ADC_CHANNEL 2

#define RPPICO_SAFE_ADC_MAX_AGGREGATE_SPS 500000u

#endif
