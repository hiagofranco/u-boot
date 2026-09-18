/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Sophgo CV181x DDR controller and PHY
 *
 * Based on the Sophgo FSBL, plat/cv181x/ddr (DDR_CFG=ddr3_1866_x16).
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#ifndef __SOPHGO_DDR_SYS_H
#define __SOPHGO_DDR_SYS_H

#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/types.h>

#define TOP_BASE	((void __iomem *)0x03000000)
#define PHYD_BASE	((void __iomem *)0x08000000)
#define DDRC_BASE	(PHYD_BASE + 0x4000)
#define PHYD_APB_BASE	(PHYD_BASE + 0x6000)
#define DDR_TOP_BASE	(PHYD_BASE + 0xa000)
#define DDR_BIST_BASE	((void __iomem *)0x08010000)

/* MBps; only the DDR3-1866 x16 register set is ported */
#define DDR_DATA_RATE		1866

#define DDR_POLL_TIMEOUT_US	5000000

/* The vendor code polls these forever; give up and report instead */
#define ddr_readl_poll(addr, val, cond)					\
({									\
	int __ret = readl_poll_timeout(addr, val, cond, DDR_POLL_TIMEOUT_US); \
									\
	if (__ret)							\
		pr_err("%s: DDR poll timeout\n", __func__);		\
	__ret;								\
})

/* SiP DDR vendor codes, from conf_info/efuse */
#define DDR_VENDOR_UNKNOWN		0x00
#define DDR_VENDOR_NY_4G		0x01
#define DDR_VENDOR_NY_2G		0x02
#define DDR_VENDOR_ESMT_1G		0x03
#define DDR_VENDOR_ESMT_512M_DDR2	0x04
#define DDR_VENDOR_ETRON_1G		0x05
#define DDR_VENDOR_ESMT_2G		0x06
#define DDR_VENDOR_PM_2G		0x07
#define DDR_VENDOR_PM_1G		0x08
#define DDR_VENDOR_ETRON_512M_DDR2	0x09
#define DDR_VENDOR_ESMT_N25_1G		0x0a
#define DDR_VENDOR_NY_N20_1G		0x0b
#define DDR_VENDOR_ESMT_N21_2G		0x0c
#define DDR_VENDOR_ESMT_N19_4G		0x0d
#define DDR_VENDOR_UNILC_N25_512M_DDR2	0x0e
#define DDR_VENDOR_UNILC_N25_1G		0x0f
#define DDR_VENDOR_UNILC_N21_2G		0x10
#define DDR_EXTERN_DDR3			0x1f

/* offset from PHYD_BASE, value */
struct ddr_reg {
	u16 off;
	u32 val;
};

/* cvx16_wdqlvl_req() lvl_mode */
#define WDQLVL_DM	0
#define WDQLVL_DQ	1
#define WDQLVL_DQ_DM	2

/* ddr_pkg_info.c */
int read_ddr_pkg_info(u8 *vendor);

/* cvx16_pinmux.c, ddr_patch_regs.c, phy_init.c, ddrc_init.c */
void cvx16_pinmux(u8 vendor);
void ddr_patch_set(u8 vendor);
void phy_init(void);
void ddrc_init(u8 vendor);
void ctrl_init_high_patch(void);
void ctrl_init_low_patch(void);
void ctrl_init_update_by_dram_size(u8 dram_cap_in_mbyte);

/* ddr_sys.c */
void cvx16_bist_wr_prbs_init(void);
void cvx16_bist_wr_sram_init(void);
int cvx16_bist_start_check(void);
void cvx16_pll_init(void);
void cvx16_set_dfi_init_start(void);
void cvx16_ddr_phy_power_on_seq1(void);
void cvx16_polling_dfi_init_start(void);
void cvx16_dfi_init_start_isr(void);
void cvx16_ddr_phy_power_on_seq3(void);
void cvx16_wait_for_dfi_init_complete(void);
void cvx16_polling_synp_normal_mode(void);
void cvx16_wrlvl_req(void);
void cvx16_rdglvl_req(void);
void cvx16_rdlvl_req(void);
void cvx16_wdqlvl_req(u32 lvl_mode);
void cvx16_clk_gating_enable(void);
void ctrl_init_detect_dram_size(u8 *dram_cap_in_mbyte);

#endif /* __SOPHGO_DDR_SYS_H */
