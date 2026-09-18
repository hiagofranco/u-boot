// SPDX-License-Identifier: GPL-2.0-only
/*
 * Sophgo CV181x SiP DDR detection
 *
 * Based on the Sophgo FSBL, plat/cv181x/ddr/ddr_pkg_info.c.
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#include "ddr_sys.h"

#define REG_CONF_INFO		(TOP_BASE + 0x4)
#define  CONF_INFO_PKG_TYPE	GENMASK(30, 28)
#define REG_GP_REG3		(TOP_BASE + 0x8c)
#define REG_EFUSE_LEAKAGE	((void __iomem *)0x03050108)
#define  EFUSE_DDR_VENDOR	GENMASK(25, 21)
#define  EFUSE_DDR_CAPACITY	GENMASK(28, 26)
#define  EFUSE_DDR_PKG		GENMASK(31, 29)

#define PKG_TYPE_FROM_EFUSE	0x4

#define DDR_CAPACITY_UNKNOWN	0
#define DDR_CAPACITY_512M	1
#define DDR_CAPACITY_1G		2
#define DDR_CAPACITY_2G		3
#define DDR_CAPACITY_4G		4

#define PKG_QFN			1
#define PKG_BGA			2

struct ddr_pkg {
	u8 vendor;
	u8 capacity;
	u8 pkg;
};

static const struct ddr_pkg ddr_pkg_types[] = {
	/* BGA 10x10, 2Gb DDR3 */
	[0x0] = { DDR_VENDOR_NY_2G, DDR_CAPACITY_2G, PKG_BGA },
	/* BGA 10x10, 4Gb DDR3 */
	[0x1] = { DDR_VENDOR_NY_4G, DDR_CAPACITY_4G, PKG_BGA },
	/* BGA 10x10, 1Gb DDR3 */
	[0x2] = { DDR_VENDOR_ESMT_1G, DDR_CAPACITY_1G, PKG_BGA },
	/* QFN 9x9, 2Gb DDR3 */
	[0x5] = { DDR_VENDOR_NY_2G, DDR_CAPACITY_2G, PKG_QFN },
	/* QFN 9x9, 1Gb DDR3 */
	[0x6] = { DDR_VENDOR_ESMT_1G, DDR_CAPACITY_1G, PKG_QFN },
	/* QFN 9x9, 512Mb DDR2 */
	[0x7] = { DDR_VENDOR_ESMT_512M_DDR2, DDR_CAPACITY_512M, PKG_QFN },
};

static bool ddr_vendor_is_ddr3(u8 vendor)
{
	switch (vendor) {
	case DDR_VENDOR_NY_4G:
	case DDR_VENDOR_NY_2G:
	case DDR_VENDOR_ESMT_1G:
	case DDR_VENDOR_ETRON_1G:
	case DDR_VENDOR_ESMT_2G:
	case DDR_VENDOR_PM_2G:
	case DDR_VENDOR_PM_1G:
	case DDR_VENDOR_ESMT_N25_1G:
	case DDR_VENDOR_NY_N20_1G:
	case DDR_VENDOR_UNILC_N25_1G:
	case DDR_VENDOR_UNILC_N21_2G:
	case DDR_VENDOR_ESMT_N21_2G:
	case DDR_VENDOR_ESMT_N19_4G:
	case DDR_EXTERN_DDR3:
		return true;
	default:
		return false;
	}
}

/*
 * Identify the DDR from the package type strap and the efuse, and record the
 * chip id in GP_REG3 for later stages. Only DDR3 is supported.
 */
int read_ddr_pkg_info(u8 *vendor)
{
	u32 conf_info = readl(REG_CONF_INFO);
	u32 efuse = readl(REG_EFUSE_LEAKAGE);
	u32 pkg_type = FIELD_GET(CONF_INFO_PKG_TYPE, conf_info);
	struct ddr_pkg ddr = { };
	u32 chip_id = 0;

	if (FIELD_GET(EFUSE_DDR_CAPACITY, efuse) == DDR_CAPACITY_UNKNOWN) {
		/* No SiP DDR: the board has external DDR3 */
		ddr.vendor = DDR_EXTERN_DDR3;
		ddr.pkg = FIELD_GET(EFUSE_DDR_PKG, efuse);
	} else if (pkg_type == PKG_TYPE_FROM_EFUSE) {
		ddr.vendor = FIELD_GET(EFUSE_DDR_VENDOR, efuse);
		ddr.capacity = FIELD_GET(EFUSE_DDR_CAPACITY, efuse);
		ddr.pkg = FIELD_GET(EFUSE_DDR_PKG, efuse);
	} else if (pkg_type < ARRAY_SIZE(ddr_pkg_types)) {
		ddr = ddr_pkg_types[pkg_type];
	}

	debug("DDR: pkg_type %x, pkg %x, capacity %x, vendor %x\n",
	      pkg_type, ddr.pkg, ddr.capacity, ddr.vendor);

	switch (ddr.capacity) {
	case DDR_CAPACITY_512M:
		chip_id = ddr.pkg == PKG_QFN ? 0x1810c : 0x1810f;
		break;
	case DDR_CAPACITY_1G:
		chip_id = ddr.pkg == PKG_QFN ? 0x1811c : 0x1811f;
		break;
	case DDR_CAPACITY_2G:
		chip_id = ddr.pkg == PKG_QFN ? 0x1812c : 0x1812f;
		break;
	case DDR_CAPACITY_4G:
		chip_id = 0x1813f;
		break;
	default:
		/* External DDR, no SiP: identify as CV1815J */
		if (ddr.vendor == DDR_EXTERN_DDR3)
			chip_id = 0x1815;
	}
	writel(chip_id, REG_GP_REG3);

	if (!ddr_vendor_is_ddr3(ddr.vendor)) {
		pr_err("DDR: unsupported DDR, pkg_type %x vendor %x\n", pkg_type, ddr.vendor);
		return -ENODEV;
	}

	*vendor = ddr.vendor;

	return 0;
}
