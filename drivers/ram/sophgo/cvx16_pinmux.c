// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV180x DDR PHY DQ/CA swap for each SiP DDR2 vendor
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv180x/ddr/cvx16_pinmux.c.
 *
 */

#include "ddr_sys.h"

struct ddr_pinmux {
	u8 vendor;
	u32 regs[12];
};

static const struct ddr_pinmux ddr_pinmux[] = {
	{ DDR_VENDOR_ESMT_512M_DDR2, {		/* N25_DDR2_512 */
		0x0c06080b, 0x070d0904, 0x00010502, 0x110a0e03,
		0x0f141610, 0x00151312, 0x00000000, 0x00000100,
		0x71840532, 0x00000006, 0x76103425, 0x00000008,
	} },
	{ DDR_VENDOR_ETRON_512M_DDR2, {		/* ETRON_DDR2_512 */
		0x070b090c, 0x04050608, 0x0e02030d, 0x110a0100,
		0x0f131614, 0x00151012, 0x00000000, 0x00000100,
		0x86014532, 0x00000007, 0x76012345, 0x00000008,
	} },
	{ DDR_VENDOR_UNILC_N25_512M_DDR2, {	/* UniIC_DDR2_512M */
		0x00040608, 0x02090b0c, 0x0e01070d, 0x0a031605,
		0x10110f14, 0x00121513, 0x00000000, 0x00000100,
		0x61437025, 0x00000008, 0x63410725, 0x00000008,
	} },
};

int cvx16_pinmux(u8 vendor)
{
	const struct ddr_pinmux *p;
	int i;

	for (p = ddr_pinmux; p < ddr_pinmux + ARRAY_SIZE(ddr_pinmux); p++) {
		if (p->vendor != vendor)
			continue;

		for (i = 0; i < ARRAY_SIZE(p->regs); i++)
			writel(p->regs[i], PHYD_BASE + i * 4);
		return 0;
	}

	pr_err("DDR: no pinmux for vendor %x\n", vendor);

	return -ENODEV;
}
