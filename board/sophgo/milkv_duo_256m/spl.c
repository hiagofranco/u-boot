// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 */

#include <asm/arch/rom_api.h>
#include <asm/io.h>
#include <asm/system.h>
#include <dm.h>
#include <hang.h>
#include <init.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>

/* Offset of u-boot.itb in the boot image read through the BootROM API */
#define CV18XX_ITB_OFFSET		0x40000

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOARD;
}

static ulong cv18xx_rom_read(struct spl_load_info *load, ulong off,
			     ulong count, void *buf)
{
	int retries = rom_api_get_number_of_retries();
	int i;

	/* The BootROM reports no retries for some boot devices */
	if (retries < 1)
		retries = 1;

	for (i = 0; i < retries; i++)
		if (rom_api_load_image(buf, CV18XX_ITB_OFFSET + off, count, i) >= 0)
			return count;

	return 0;
}

static int cv18xx_spl_romapi_load(struct spl_image_info *spl_image,
				  struct spl_boot_device *bootdev)
{
	struct spl_load_info load;

	spl_load_init(&load, cv18xx_rom_read, NULL, 512);

	return spl_load(spl_image, bootdev, &load, 0, 0);
}

SPL_LOAD_IMAGE_METHOD("ROMAPI", 0, BOOT_DEVICE_BOARD, cv18xx_spl_romapi_load);

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	ret = spl_early_init();
	if (ret)
		hang();

	preloader_console_init();

	/* Probe the CPU so that the RISC-V timer is available */
	ret = uclass_get_device(UCLASS_CPU, 0, &dev);
	if (ret)
		panic("Failed to probe CPU: %d\n", ret);

	riscv_cpu_setup();

	/* DDR training runs from the XTAL, before the PLLs are enabled */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("DRAM init failed: %d\n", ret);
}
