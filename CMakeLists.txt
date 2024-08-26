cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(bruteforce)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(${PROJECT_NAME}
    main.cpp
    usb_descriptors.c
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        .
        )

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        tinyusb_device
        tinyusb_board
        )

add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
)


pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# add_executable(picow_access_point_poll
#         picow_access_point.c
#         dhcpserver/dhcpserver.c
#         dnsserver/dnsserver.c
#         )
# target_include_directories(picow_access_point_poll PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}
#         ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
#         ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
#         ${CMAKE_CURRENT_LIST_DIR}/dnsserver
#         )
# target_link_libraries(picow_access_point_poll
#         pico_cyw43_arch_lwip_poll
#         pico_stdlib
#         )
# pico_add_extra_outputs(picow_access_point_poll)
