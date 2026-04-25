// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		hang();

	preloader_console_init();

	printf("Hello from U-Boot SPL on Milk-V Duo 256M!\n");

	hang();
}
