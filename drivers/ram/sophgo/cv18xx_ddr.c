// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV181x DDR3 driver for SPL
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv181x/ddr/ddr_sys_bring_up.c.
 */

#include <config.h>
#include <dm.h>
#include <linux/sizes.h>
#include <ram.h>

#include "ddr_sys.h"

struct cv18xx_ddr_priv {
	struct ram_info info;
};

static int cv18xx_ddr_bist(const char *step, bool sram)
{
	int ret;

	if (sram)
		cvx16_bist_wr_sram_init();
	else
		cvx16_bist_wr_prbs_init();

	ret = cvx16_bist_start_check();
	if (ret)
		debug("DDR: %s BIST failed after %s: %d\n",
		      sram ? "SRAM" : "PRBS", step, ret);

	return ret;
}

static int cv18xx_ddr_probe(struct udevice *dev)
{
	struct cv18xx_ddr_priv *priv = dev_get_priv(dev);
	u8 vendor, dram_cap;
	int ret;

	ret = read_ddr_pkg_info(&vendor);
	if (ret)
		return ret;

	cvx16_pll_init();
	ddrc_init(vendor);

	/* Release the DDRC soft reset */
	writel(0x0, DDR_TOP_BASE + 0x20);

	/*
	 * AXI QoS: M1 VIP realtime 0xa, M2 VIP offline 0x8, M3 CPU 0x7,
	 * M4 TPU 0x0, M5 video codec 0x9, M6 high speed peripherals 0x2
	 */
	writel(0x007788aa, TOP_BASE + 0x1d8);
	writel(0x00002299, TOP_BASE + 0x1dc);

	phy_init();
	writel(0x02620504, PHYD_BASE + 0xa4);
	ret = cvx16_pinmux(vendor);
	if (ret)
		return ret;
	ddr_patch_set(vendor);

	cvx16_set_dfi_init_start();
	cvx16_ddr_phy_power_on_seq1();
	cvx16_polling_dfi_init_start();
	cvx16_dfi_init_start_isr();
	cvx16_ddr_phy_power_on_seq3();
	cvx16_wait_for_dfi_init_complete();
	cvx16_polling_synp_normal_mode();
	cv18xx_ddr_bist("init", false);

	ctrl_init_low_patch();
	cvx16_wrlvl_req();
	cv18xx_ddr_bist("wrlvl", false);
	cvx16_rdglvl_req();
	cv18xx_ddr_bist("rdglvl", false);

	cvx16_wdqlvl_req(WDQLVL_DQ_DM);
	cvx16_wdqlvl_req(WDQLVL_DQ);
	cvx16_wdqlvl_req(WDQLVL_DM);
	cv18xx_ddr_bist("wdqlvl", false);

	/* param_phyd_pirdlvl_capture_cnt = 1 */
	clrsetbits_le32(PHYD_BASE + 0x8c, GENMASK(7, 4), FIELD_PREP(GENMASK(7, 4), 1));
	cvx16_rdlvl_req();
	cv18xx_ddr_bist("rdlvl", false);

	ctrl_init_high_patch();
	ret = ctrl_init_detect_dram_size(&dram_cap);
	if (ret)
		return ret;
	ctrl_init_update_by_dram_size(dram_cap);
	cvx16_clk_gating_enable();

	ret = cv18xx_ddr_bist("training", false) ?: cv18xx_ddr_bist("training", true);
	if (ret) {
		pr_err("DDR: BIST failed\n");
		return ret;
	}

	priv->info.base = CFG_SYS_SDRAM_BASE;
	priv->info.size = (size_t)SZ_1M << dram_cap;

	return 0;
}

static int cv18xx_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	struct cv18xx_ddr_priv *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static const struct ram_ops cv18xx_ddr_ops = {
	.get_info = cv18xx_ddr_get_info,
};

static const struct udevice_id cv18xx_ddr_ids[] = {
	{ .compatible = "sophgo,sg2002-ddr" },
	{ }
};

U_BOOT_DRIVER(cv18xx_ddr) = {
	.name = "cv18xx_ddr",
	.id = UCLASS_RAM,
	.of_match = cv18xx_ddr_ids,
	.ops = &cv18xx_ddr_ops,
	.probe = cv18xx_ddr_probe,
	.priv_auto = sizeof(struct cv18xx_ddr_priv),
};
