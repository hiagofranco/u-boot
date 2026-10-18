// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV180x SiP DDR detection
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv180x/ddr/ddr_pkg_info.c.
 *
 */

#include "ddr_sys.h"

#define REG_CONF_INFO		(TOP_BASE + 0x4)
#define  CONF_INFO_PKG_TYPE	GENMASK(30, 28)
#define REG_GP_REG3		(TOP_BASE + 0x8c)
#define REG_EFUSE_LEAKAGE	((void __iomem *)0x03050108)
#define  EFUSE_DDR_VENDOR	GENMASK(25, 21)
#define  EFUSE_DDR_CAPACITY	GENMASK(28, 26)
#define  EFUSE_DDR_PKG		GENMASK(31, 29)

/* Second source parts, described by the efuse */
#define PKG_TYPE_FROM_EFUSE_0	0x0
#define PKG_TYPE_FROM_EFUSE_1	0x4

#define DDR_CAPACITY_UNKNOWN	0
#define DDR_CAPACITY_512M	1
#define DDR_CAPACITY_1G		2
#define DDR_CAPACITY_2G		3
#define DDR_CAPACITY_4G		4

#define PKG_QFN88		1
#define PKG_BGA			2
#define PKG_QFN68		3

struct ddr_pkg {
	u8 vendor;
	u8 capacity;
	u8 pkg;
};

static const struct ddr_pkg ddr_pkg_types[] = {
	/* QFN68 7x7, 1Gb DDR3 */
	[0x2] = { DDR_VENDOR_ESMT_1G, DDR_CAPACITY_1G, PKG_QFN68 },
	/* QFN68 7x7, 512Mb DDR2 (25nm) */
	[0x3] = { DDR_VENDOR_ESMT_512M_DDR2, DDR_CAPACITY_512M, PKG_QFN68 },
	/* QFN88 9x9, 1Gb DDR3 */
	[0x6] = { DDR_VENDOR_ESMT_1G, DDR_CAPACITY_1G, PKG_QFN88 },
	/* QFN88 9x9, 512Mb DDR2 (25nm) */
	[0x7] = { DDR_VENDOR_ESMT_512M_DDR2, DDR_CAPACITY_512M, PKG_QFN88 },
};

static bool ddr_vendor_is_ddr2(u8 vendor)
{
	switch (vendor) {
	case DDR_VENDOR_ESMT_512M_DDR2:
	case DDR_VENDOR_ETRON_512M_DDR2:
	case DDR_VENDOR_UNILC_N25_512M_DDR2:
		return true;
	default:
		return false;
	}
}

/*
 * Identify the DDR from the package type strap and the efuse, and record the
 * chip id in GP_REG3 for later stages. Only DDR2 is supported.
 */
int read_ddr_pkg_info(u8 *vendor)
{
	u32 conf_info = readl(REG_CONF_INFO);
	u32 efuse = readl(REG_EFUSE_LEAKAGE);
	u32 pkg_type = FIELD_GET(CONF_INFO_PKG_TYPE, conf_info);
	struct ddr_pkg ddr = { };
	u32 chip_id = 0;

	if (pkg_type == PKG_TYPE_FROM_EFUSE_0 || pkg_type == PKG_TYPE_FROM_EFUSE_1) {
		ddr.vendor = FIELD_GET(EFUSE_DDR_VENDOR, efuse);
		ddr.capacity = FIELD_GET(EFUSE_DDR_CAPACITY, efuse);
		ddr.pkg = FIELD_GET(EFUSE_DDR_PKG, efuse);
	} else if (pkg_type < ARRAY_SIZE(ddr_pkg_types)) {
		ddr = ddr_pkg_types[pkg_type];
	}

	debug("DDR: pkg_type %x, pkg %x, capacity %x, vendor %x\n",
	      pkg_type, ddr.pkg, ddr.capacity, ddr.vendor);

	/* CV1800B/CV1801B on QFN68, CV1800C/CV1801C on QFN88 */
	switch (ddr.capacity) {
	case DDR_CAPACITY_512M:
		chip_id = ddr.pkg == PKG_QFN88 ? 0x1800c : 0x1800b;
		break;
	case DDR_CAPACITY_1G:
		chip_id = ddr.pkg == PKG_QFN88 ? 0x1801c : 0x1801b;
		break;
	default:
		/* No known CV180x chip id for the other capacities */
		break;
	}
	writel(chip_id, REG_GP_REG3);

	if (!ddr_vendor_is_ddr2(ddr.vendor)) {
		pr_err("DDR: unsupported DDR, pkg_type %x vendor %x\n", pkg_type, ddr.vendor);
		return -ENODEV;
	}

	*vendor = ddr.vendor;

	return 0;
}
