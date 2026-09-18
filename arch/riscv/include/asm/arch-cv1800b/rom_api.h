/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#ifndef __ASM_ARCH_CV1800B_ROM_API_H
#define __ASM_ARCH_CV1800B_ROM_API_H

#include <linux/types.h>

#define CV1800B_ROM_API_BASE		0x04418000
#define CV1800B_ROM_API_LOAD_IMAGE	(CV1800B_ROM_API_BASE + 0x60)
#define CV1800B_ROM_API_GET_RETRIES	(CV1800B_ROM_API_BASE + 0xc0)

/*
 * Read @size bytes at @offset of the boot image from the boot device the
 * BootROM booted from. Returns a negative value on failure.
 */
static inline int rom_api_load_image(void *buf, u32 offset, size_t size,
				     int retry)
{
	int (*load_image)(void *buf, u32 offset, size_t size, int retry) =
		(void *)CV1800B_ROM_API_LOAD_IMAGE;

	return load_image(buf, offset, size, retry);
}

/* Number of retries the BootROM allows for the current boot device */
static inline int rom_api_get_number_of_retries(void)
{
	int (*get_retries)(void) = (void *)CV1800B_ROM_API_GET_RETRIES;

	return get_retries();
}

#endif /* __ASM_ARCH_CV1800B_ROM_API_H */
