load("@stm32//stm32f4:rules.bzl", "stm32f4_binary", "stm32f4_hal_library")
load("@stm32//stm32f4:rules_middlewares.bzl", "stm32_usb_device_library")
load("@stm32//tools/arm_compiler:raw_binary.bzl", "raw_binary", "hex_binary")

stm32f4_hal_library(
    name = "hal_library",
    hdrs = [
        "Inc/main.h",
        "Inc/stm32f4xx_hal_conf.h",
        "Inc/stm32f4xx_it.h",
    ],
    include_path = "firmware/stm/Inc",
    modules = [
        "tim",
        "uart",
        "pdc",
        "gpio",
        "dma",
        "rcc",
        "flash",
        "pwr",
        "cortex",
        "ll_usb",
    ],
    processor = "STM32F407xx",
)

stm32_usb_device_library(
    name = "usb_cdc_lib",
    srcs = [
        "Src/usb_device.c",
        "Src/usbd_cdc_if.c",
        "Src/usbd_conf.c",
        "Src/usbd_desc.c",
    ],
    hdrs = [
        "Inc/events.h",
        "Inc/usb_device.h",
        "Inc/usbd_cdc_if.h",
        "Inc/usbd_conf.h",
        "Inc/usbd_desc.h",
    ],
    include_path = "firmware/stm/Inc",
    usb_class = "cdc",
    deps = [":hal_library"],
)

stm32f4_binary(
    name = "firmware_elf",
    srcs = glob(
        [
            "Src/*.cpp",
            "Src/*.c",
        ],
    ) + [
        "Inc/comm_ascii.h",
        "Inc/comm_protobuf.h",
        "Inc/command_handler.h",
        "Inc/events.h",
        "Inc/main.h",
        "Inc/motor.h",
        "Inc/servo.h",
    ],
    include_path = "firmware/stm/Inc",
    linker_script = "STM32F407VGTx_FLASH.ld",
    processor = "STM32F407xx",
    use_hal = True,
    deps = [
        ":hal_library",
        ":usb_cdc_lib",
        "//packages/hal/proto:nanopb_vcu",
    ],
)

raw_binary(
    name = "firmware_bin",
    src = ":firmware_elf",
)

hex_binary(
    name = "firmware_hex",
    src = ":firmware_elf",
)
