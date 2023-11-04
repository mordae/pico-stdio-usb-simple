/*
 * Copyright (c) 2023 Jan Hamal Dvořák
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <pico/stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern stdio_driver_t stdio_usb;

/*
 * Initialize simple USB stdio and add it to the current set of drivers.
 *
 * Returns true if the USB CDC was initialized, false if an error occurred.
 */
bool stdio_usb_init(void);

/*
 * Check if there is an active stdio CDC connection to a host.
 *
 * Returns true if stdio is connected over CDC.
 */
bool stdio_usb_connected(void);

/*
 * Take over control of the USB stack.
 *
 * You must make sure to call tud_task() as needed yourself.
 */
void stdio_usb_lock(void);

/*
 * Release control of the USB stack.
 *
 * From now on, tud_task() will be ran from an periodic interrupt.
 */
void stdio_usb_unlock(void);

#ifdef __cplusplus
}
#endif
