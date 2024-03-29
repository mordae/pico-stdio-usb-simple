/**
 * Copyright (c) 2023 Jan Hamal Dvořák
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/bootrom.h>
#include <pico/usb_reset_interface.h>

#include <hardware/watchdog.h>

#include <tusb.h>
#include <device/usbd_pvt.h>

static uint8_t itf_num;

static void resetd_init(void)
{
}

static void resetd_reset(uint8_t __unused rhport)
{
	itf_num = 0;
}

static uint16_t resetd_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc,
			    uint16_t max_len)
{
	TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
			  RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
			  RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol,
		  0);

	uint16_t const drv_len = sizeof(tusb_desc_interface_t);
	TU_VERIFY(max_len >= drv_len, 0);

	itf_num = itf_desc->bInterfaceNumber;
	return drv_len;
}

// Support for parameterized reset via vendor interface control request
static bool resetd_control_xfer_cb(uint8_t __unused rhport, uint8_t stage,
				   tusb_control_request_t const *request)
{
	// nothing to do with DATA & ACK stage
	if (stage != CONTROL_STAGE_SETUP)
		return true;

	if (request->wIndex == itf_num) {
		if (RESET_REQUEST_BOOTSEL == request->bRequest)
			reset_usb_boot(0, 0);

		if (RESET_REQUEST_FLASH == request->bRequest) {
			watchdog_reboot(0, 0, 100);
			return true;
		}
	}

	return false;
}

static bool resetd_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr,
			   xfer_result_t __unused result, uint32_t __unused xferred_bytes)
{
	return true;
}

static usbd_class_driver_t const driver = {
#if CFG_TUSB_DEBUG >= 2
	.name = "RESET",
#endif
	.init = resetd_init,
	.reset = resetd_reset,
	.open = resetd_open,
	.control_xfer_cb = resetd_control_xfer_cb,
	.xfer_cb = resetd_xfer_cb,
	.sof = NULL,
};

// Implement callback to add our custom driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
	*driver_count = 1;
	return &driver;
}

// Support for default BOOTSEL reset by changing baud rate
void tud_cdc_line_coding_cb(__unused uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
	if (1200 == p_line_coding->bit_rate)
		reset_usb_boot(0, 0);
}
