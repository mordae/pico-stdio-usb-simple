if (TARGET tinyusb_device_unmarked)
  pico_add_library(pico_stdio_usb_simple)

  target_include_directories(
    pico_stdio_usb_simple_headers
    INTERFACE
      ${CMAKE_CURRENT_LIST_DIR}/include
  )

  target_sources(pico_stdio_usb_simple
    INTERFACE
      ${CMAKE_CURRENT_LIST_DIR}/stdio_usb_simple.c
      ${CMAKE_CURRENT_LIST_DIR}/reset_interface.c
      ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
  )

  target_link_libraries(
    pico_stdio_usb_simple
    INTERFACE
      tinyusb_device_unmarked
      pico_usb_reset_interface
      pico_unique_id
  )
endif()
