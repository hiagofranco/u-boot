// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV180x DDR2-1333 x16 PHY patch settings
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv180x/ddr/ddr_config/ddr2_1333_x16/ddr_patch_regs.c.
 *
 */

#include "ddr_sys.h"

static const struct ddr_reg ddr_patch_regs[] = {
	/* BYTE0 DQ driving */
	{ 0xa38, 0x00000808 },
	/* BYTE1 DQ driving */
	{ 0xa78, 0x00000808 },

	/* BYTE0 RX DQ deskew */
	{ 0xb00, 0x03010504 },
	{ 0xb04, 0x02030203 },
	/* BYTE0 DQ8 deskew [6:0], neg DQS [15:8], pos DQS [23:16] */
	{ 0xb08, 0x002a4204 },

	/* BYTE1 RX DQ deskew */
	{ 0xb30, 0x08030803 },
	{ 0xb34, 0x02070205 },
	/* BYTE1 DQ8 deskew [6:0], neg DQS [15:8], pos DQS [23:16] */
	{ 0xb38, 0x002b4003 },
};

void ddr_patch_set(void)
{
	const struct ddr_reg *r;

	for (r = ddr_patch_regs; r < ddr_patch_regs + ARRAY_SIZE(ddr_patch_regs); r++)
		writel(r->val, PHYD_BASE + r->off);
}
