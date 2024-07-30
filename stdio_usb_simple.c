/**
 * Copyright (c) 2023 Jan Hamal Dvořák
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/binary_info.h>
#include <pico/mutex.h>
#include <pico/stdio/driver.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#include <tusb.h>

#define PERIODIC_TASK_INTERVAL_US 1000

static mutex_t mutex;

void stdio_usb_lock(void)
{
	mutex_enter_blocking(&mutex);
}

void stdio_usb_unlock(void)
{
	mutex_exit(&mutex);
}

static void stdio_usb_out_chars(const char *buf, int length)
{
	/* Will deadlock if called recursively from an interrupt handler. */
	stdio_usb_lock();

	uint32_t sofar = 0;

	while (length > 0) {
		tud_task();

		if (!stdio_usb_connected()) {
			/* We were disconnected, give up. */
			goto unlock;
		}

		int avail = tud_cdc_write_available();

		if (avail) {
			if (avail > length)
				avail = length;

			int written = tud_cdc_write(buf + sofar, avail);

			if (written < 0)
				goto unlock;

			sofar += written;
			length -= written;

			tud_cdc_write_flush();
		} else {
			tud_task();
		}
	}

	tud_task();

unlock:
	stdio_usb_unlock();
}

static int stdio_usb_in_chars(char *buf, int length)
{
	if (!stdio_usb_connected() || !tud_cdc_available())
		return PICO_ERROR_NO_DATA;

	stdio_usb_lock();
	tud_task();

	int res = PICO_ERROR_NO_DATA;
	int avail = tud_cdc_available();

	if (avail > length)
		avail = length;

	if (0 == avail) {
		tud_task();
		goto unlock;
	}

	res = tud_cdc_read(buf, avail);
	if (0 == res)
		res = PICO_ERROR_NO_DATA;

unlock:
	stdio_usb_unlock();
	return res;
}

static void stdio_usb_out_flush(void)
{
	while (true) {
		if (!stdio_usb_connected())
			return;

		int avail = tud_cdc_write_available();

		if (CFG_TUD_CDC_TX_BUFSIZE == avail)
			break;

		tud_task();
	}
}

stdio_driver_t stdio_usb = {
	.out_chars = stdio_usb_out_chars,
	.in_chars = stdio_usb_in_chars,
	.out_flush = stdio_usb_out_flush,
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
	.crlf_enabled = PICO_STDIO_DEFAULT_CRLF,
#endif
};

static int64_t periodic_task(alarm_id_t __unused id, void __unused *data)
{
	if (mutex_try_enter(&mutex, NULL)) {
		tud_task();
		mutex_exit(&mutex);
	}

	return -PERIODIC_TASK_INTERVAL_US;
}

bool stdio_usb_init(void)
{
#if !PICO_NO_BI_STDIO_USB
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	bi_decl_if_func_used(bi_program_feature("USB stdin / stdout"));
#pragma GCC diagnostic pop
#endif

	mutex_init(&mutex);

	tusb_init();
	add_alarm_in_us(0, periodic_task, NULL, true);

	stdio_set_driver_enabled(&stdio_usb, true);
	return true;
}

bool stdio_usb_connected(void)
{
	return tud_cdc_connected();
}
