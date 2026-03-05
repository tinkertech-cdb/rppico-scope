add_executable(rppico_scope_fw
    main.c
    protocol_v1_crc.c
    protocol_engine.c
)

target_include_directories(rppico_scope_fw PRIVATE
    ${CMAKE_SOURCE_DIR}/../protocol/include
)

target_compile_definitions(rppico_scope_fw PRIVATE
    RPPICO_USB_VID=${RPPICO_USB_VID}
    RPPICO_USB_PID=${RPPICO_USB_PID}
    RPPICO_USB_MANUFACTURER="${RPPICO_USB_MANUFACTURER}"
    RPPICO_USB_PRODUCT="${RPPICO_USB_PRODUCT}"
)

target_link_libraries(rppico_scope_fw
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_pio
    hardware_pwm
)

# USB console on by default for bring-up logs.
pico_enable_stdio_usb(rppico_scope_fw 1)
pico_enable_stdio_uart(rppico_scope_fw 0)

pico_add_extra_outputs(rppico_scope_fw)
