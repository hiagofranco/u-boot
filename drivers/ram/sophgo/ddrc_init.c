// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV180x DDR2-1333 x16 controller settings
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv180x/ddr/ddr_config/ddr2_1333_x16/ddrc_init.c.
 *
 */

#include "ddr_sys.h"

#define DDRC_ADDRMAP(n)		(0x200 + (n) * 4)
#define DDRC_NUM_ADDRMAP	12

/* ADDRMAP0 to ADDRMAP11 */
static void ddrc_set_addrmap(const u32 *map)
{
	int i;

	for (i = 0; i < DDRC_NUM_ADDRMAP; i++)
		writel(map[i], DDRC_BASE + DDRC_ADDRMAP(i));
}

/* R[17:13] B[2] R[12:0] B[1:0] C[9:0], from 0.5 Gbit to 4 Gbit */
static const u32 ddrc_addrmap_etron[DDRC_NUM_ADDRMAP] = {
	0x00001f1f, 0x00141414, 0x00000000, 0x1f000000,
	0x00001f1f, 0x030f0303, 0x06060603, 0x00000606,
	0x00003f3f, 0x03030303, 0x03030303, 0x001f1f03,
};

static const u32 ddrc_addrmap_esmt[DDRC_NUM_ADDRMAP] = {
	0x00001f1f, 0x00140707, 0x00000000, 0x1f000000,
	0x00001f1f, 0x050f0505, 0x06060605, 0x00000606,
	0x00003f3f, 0x05050505, 0x05050505, 0x001f1f05,
};

void ddrc_init(u8 vendor)
{
	bool etron = vendor == DDR_VENDOR_ETRON_512M_DDR2;

	/*
	 * PATCH0.use_blk_ext:0:2:=0x1
	 * PATCH0.dis_auto_ref_cnt_fix:2:1:=0x0
	 * PATCH0.dis_auto_ref_algn_to_8:3:1:=0x0
	 * PATCH0.starve_stall_at_dfi_ctrlupd:4:1:=0x1
	 * PATCH0.starve_stall_at_abr:5:1:=0x1
	 * PATCH0.dis_rdwr_switch_at_abr:6:1:=0x1
	 * PATCH0.dfi_wdata_same_to_axi:7:1:=0x0
	 * PATCH0.pagematch_limit_threshold:8:3=0x3
	 * PATCH0.qos_sel:12:2:=0x2
	 * PATCH0.burst_rdwr_xpi:16:4:=0x4
	 * PATCH0.always_critical_when_urgent_hpr:20:1:=0x1
	 * PATCH0.always_critical_when_urgent_lpr:21:1:=0x1
	 * PATCH0.always_critical_when_urgent_wr:22:1:=0x1
	 * PATCH0.disable_hif_rcmd_stall_path:24:1:=0x1
	 * PATCH0.disable_hif_wcmd_stall_path:25:1:=0x1
	 * PATCH0.derate_sys_en:29:1:=0x1
	 * PATCH0.ref_4x_sys_high_temp:30:1:=0x1
	 */
	writel(0x63746371, DDRC_BASE + 0xc);
	/*
	 * PATCH1.ref_adv_stop_threshold:0:7:=0x0
	 * PATCH1.ref_adv_dec_threshold:8:7:=0x0
	 * PATCH1.ref_adv_max:16:7:=0x0
	 * PATCH1.burst_rdwr_wr_xpi:24:4:=0x4
	 * PATCH1.use_blk_extend:28:2:=0x1
	 */
	writel(0x14000000, DDRC_BASE + 0x44);
	/*
	 * PATCH5.vpr_fix:0:1:=0x1
	 * PATCH5.vpw_fix:1:1:=0x1
	 */
	writel(0x00000003, DDRC_BASE + 0x6c);
	/*
	 * PATCH4.t_phyd_rden:16:6=0x1d (ETRON), 0x1c (others)
	 * PATCH4.phyd_rd_clk_stop:23:1=0x1
	 * PATCH4.t_phyd_wren:24:6=0x18 (ETRON), 0x17 (others)
	 * PATCH4.phyd_wr_clk_stop:31:1=0x1
	 */
	writel(etron ? 0x989d0000 : 0x979c0000, DDRC_BASE + 0x148);
	writel(0x81041400, DDRC_BASE);
	writel(0x00000000, DDRC_BASE + 0x30);
	writel(0x006a0001, DDRC_BASE + 0x34);
	writel(0x00020000, DDRC_BASE + 0x38);
	writel(0x00201070, DDRC_BASE + 0x50);
	writel(0x00000000, DDRC_BASE + 0x60);
	writel(etron ? 0x00510023 : 0x0051002c, DDRC_BASE + 0x64);
	writel(0x00010043, DDRC_BASE + 0xd0);
	writel(0x00000000, DDRC_BASE + 0xd4);
	if (etron) {
		writel(0x03030040, DDRC_BASE + 0xdc);
		writel(0x00800000, DDRC_BASE + 0xe0);
		writel(0x0b01160f, DDRC_BASE + 0x100);
		writel(0x00020414, DDRC_BASE + 0x104);
		writel(0x04050409, DDRC_BASE + 0x108);
		writel(0x00001004, DDRC_BASE + 0x10c);
		writel(0x05020405, DDRC_BASE + 0x110);
		writel(0x01010202, DDRC_BASE + 0x114);
		writel(0x00000503, DDRC_BASE + 0x120);
	} else {
		writel(0x03730040, DDRC_BASE + 0xdc);
		writel(0x00800000, DDRC_BASE + 0xe0);
		writel(0x0a011610, DDRC_BASE + 0x100);
		writel(0x00030414, DDRC_BASE + 0x104);
		writel(0x03040408, DDRC_BASE + 0x108);
		writel(0x00003004, DDRC_BASE + 0x10c);
		writel(0x05020406, DDRC_BASE + 0x110);
		writel(0x01010303, DDRC_BASE + 0x114);
		writel(0x00000503, DDRC_BASE + 0x120);
	}
	/* phyd related */
	/*
	 * DFITMG0.dfi_t_ctrl_delay:24:5:=0x4
	 * DFITMG0.dfi_rddata_use_dfi_phy_clk:23:1:=0x1
	 * DFITMG0.dfi_t_rddata_en:16:7:=0x7 (ETRON), 0x5 (others)
	 * DFITMG0.dfi_wrdata_use_dfi_phy_clk:15:1:=0x1
	 * DFITMG0.dfi_tphy_wrdata:8:6:=0x3
	 * DFITMG0.dfi_tphy_wrlat:0:6:=0x4 (ETRON), 0x2 (others)
	 */
	writel(etron ? 0x04878304 : 0x04858302, DDRC_BASE + 0x190);
	/*
	 * DFITMG1.dfi_t_cmd_lat:28:4:=0x0
	 * DFITMG1.dfi_t_parin_lat:24:2:=0x0
	 * DFITMG1.dfi_t_wrdata_delay:16:5:=0x7
	 * DFITMG1.dfi_t_dram_clk_disable:8:5:=0x1
	 * DFITMG1.dfi_t_dram_clk_enable:0:5:=0x2
	 */
	writel(0x00070102, DDRC_BASE + 0x194);
	/*
	 * DFILPCFG0.dfi_tlp_resp:24:5:=0x7
	 * DFILPCFG0.dfi_lp_wakeup_dpd:20:4:=0xc
	 * DFILPCFG0.dfi_lp_en_dpd:16:1:=0x1
	 * DFILPCFG0.dfi_lp_wakeup_sr:12:4:=0x3
	 * DFILPCFG0.dfi_lp_en_sr:8:1:=0x1
	 * DFILPCFG0.dfi_lp_wakeup_pd:4:4:=0x2
	 * DFILPCFG0.dfi_lp_en_pd:0:1:=0x1
	 */
	writel(0x07c13121, DDRC_BASE + 0x198);
	/*
	 * DFILPCFG1.dfi_lp_wakeup_mpsm:4:4:=0x2
	 * DFILPCFG1.dfi_lp_en_mpsm:0:1:=0x1
	 */
	writel(0x00000021, DDRC_BASE + 0x19c);
	writel(0xc0400018, DDRC_BASE + 0x1a0);
	writel(0x00fe00ff, DDRC_BASE + 0x1a4);
	writel(0x80000000, DDRC_BASE + 0x1a8);
	writel(0x000002c1, DDRC_BASE + 0x1b0);
	writel(0x00000001, DDRC_BASE + 0x1c0);
	writel(0x00000001, DDRC_BASE + 0x1c4);
	/* address map, auto gen. */
	ddrc_set_addrmap(etron ? ddrc_addrmap_etron : ddrc_addrmap_esmt);
	writel(etron ? 0x07030710 : 0x07010708, DDRC_BASE + 0x240);
	writel(0x00000000, DDRC_BASE + 0x244);
	/*
	 * SCHED.opt_vprw_sch:31:1:=0x0
	 * SCHED.rdwr_idle_gap:24:7:=0x0
	 * SCHED.go2critical_hysteresis:16:8:=0x0
	 * SCHED.lpddr4_opt_act_timing:15:1:=0x0
	 * SCHED.lpr_num_entries:8:7:=0x3f
	 * SCHED.autopre_rmw:7:1:=0x1
	 * SCHED.dis_opt_ntt_by_pre:6:1:=0x0
	 * SCHED.dis_opt_ntt_by_act:5:1:=0x0
	 * SCHED.opt_wrcam_fill_level:4:1:=0x0
	 * SCHED.rdwr_switch_policy_sel:3:1:=0x0
	 * SCHED.pageclose:2:1:=0x1
	 * SCHED.prefer_write:1:1:=0x0
	 * SCHED.dis_opt_wrecc_collision_flush:0:1:=0x1
	 */
	writel(0x00003f85, DDRC_BASE + 0x250);
	/*
	 * SCHED1.page_hit_limit_rd:28:3:=0x0
	 * SCHED1.page_hit_limit_wr:24:3:=0x0
	 * SCHED1.visible_window_limit_rd:20:3:=0x0
	 * SCHED1.visible_window_limit_wr:16:3:=0x0
	 * SCHED1.delay_switch_write:12:4:=0x0
	 * SCHED1.pageclose_timer:0:8:=0x20
	 */
	writel(0x00000020, DDRC_BASE + 0x254);
	/*
	 * PERFHPR1.hpr_xact_run_length:24:8:=0x10
	 * PERFHPR1.hpr_max_starve:0:16:=0xa8
	 */
	writel(0x100000a8, DDRC_BASE + 0x25c);
	/*
	 * PERFLPR1.lpr_xact_run_length:24:8:=0x10
	 * PERFLPR1.lpr_max_starve:0:16:=0xa8
	 */
	writel(0x100000a8, DDRC_BASE + 0x264);
	/*
	 * PERFWR1.w_xact_run_length:24:8:=0x10
	 * PERFWR1.w_max_starve:0:16:=0xa8
	 */
	writel(0x100000a8, DDRC_BASE + 0x26c);
	/*
	 * DBG0.dis_max_rank_wr_opt:7:1:=0x0
	 * DBG0.dis_max_rank_rd_opt:6:1:=0x0
	 * DBG0.dis_collision_page_opt:4:1:=0x0
	 * DBG0.dis_act_bypass:2:1:=0x0
	 * DBG0.dis_rd_bypass:1:1:=0x0
	 * DBG0.dis_wc:0:1:=0x0
	 */
	writel(0x00000000, DDRC_BASE + 0x300);
	/*
	 * DBG1.dis_hif:1:1:=0x0
	 * DBG1.dis_dq:0:1:=0x0
	 */
	writel(0x00000000, DDRC_BASE + 0x304);
	writel(0x00000000, DDRC_BASE + 0x30c);
	/* SWCTL.sw_done:0:1:=0x1 */
	writel(0x00000001, DDRC_BASE + 0x320);
	/*
	 * POISONCFG.rd_poison_intr_clr:24:1:=0x0
	 * POISONCFG.rd_poison_intr_en:20:1:=0x0
	 * POISONCFG.rd_poison_slverr_en:16:1:=0x0
	 * POISONCFG.wr_poison_intr_clr:8:1:=0x0
	 * POISONCFG.wr_poison_intr_en:4:1:=0x0
	 * POISONCFG.wr_poison_slverr_en:0:1:=0x0
	 */
	writel(0x00000000, DDRC_BASE + 0x36c);
	/*
	 * PCCFG.dch_density_ratio:12:2:=0x0
	 * PCCFG.bl_exp_mode:8:1:=0x0
	 * PCCFG.pagematch_limit:4:1:=0x1
	 * PCCFG.go2critical_en:0:1:=0x1
	 */
	writel(0x00000011, DDRC_BASE + 0x400);
	/*
	 * PCFGR_0.rdwr_ordered_en:16:1:=0x0
	 * PCFGR_0.rd_port_pagematch_en:14:1:=0x1
	 * PCFGR_0.rd_port_urgent_en:13:1:=0x1
	 * PCFGR_0.rd_port_aging_en:12:1:=0x0
	 * PCFGR_0.read_reorder_bypass_en:11:1:=0x0
	 * PCFGR_0.rd_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x404);
	/*
	 * PCFGW_0.wr_port_pagematch_en:14:1:=0x1
	 * PCFGW_0.wr_port_urgent_en:13:1:=0x1
	 * PCFGW_0.wr_port_aging_en:12:1:=0x0
	 * PCFGW_0.wr_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x408);
	/* PCTRL_0.port_en:0:1:=0x1 */
	writel(0x00000001, DDRC_BASE + 0x490);
	/*
	 * PCFGQOS0_0.rqos_map_region2:24:8:=0x0
	 * PCFGQOS0_0.rqos_map_region1:20:4:=0x0
	 * PCFGQOS0_0.rqos_map_region0:16:4:=0x0
	 * PCFGQOS0_0.rqos_map_level2:8:8:=0x0
	 * PCFGQOS0_0.rqos_map_level1:0:8:=0x7
	 */
	writel(0x00000007, DDRC_BASE + 0x494);
	/*
	 * PCFGQOS1_0.rqos_map_timeoutr:16:16:=0x0
	 * PCFGQOS1_0.rqos_map_timeoutb:0:16:=0x6a
	 */
	writel(0x0000006a, DDRC_BASE + 0x498);
	/*
	 * PCFGWQOS0_0.wqos_map_region2:24:8:=0x0
	 * PCFGWQOS0_0.wqos_map_region1:20:4:=0x0
	 * PCFGWQOS0_0.wqos_map_region0:16:4:=0x0
	 * PCFGWQOS0_0.wqos_map_level2:8:8:=0xe
	 * PCFGWQOS0_0.wqos_map_level1:0:8:=0x7
	 */
	writel(0x00000e07, DDRC_BASE + 0x49c);
	/*
	 * PCFGWQOS1_0.wqos_map_timeout2:16:16:=0x1a8
	 * PCFGWQOS1_0.wqos_map_timeout1:0:16:=0x1a8
	 */
	writel(0x01a801a8, DDRC_BASE + 0x4a0);
	/*
	 * PCFGR_1.rdwr_ordered_en:16:1:=0x0
	 * PCFGR_1.rd_port_pagematch_en:14:1:=0x1
	 * PCFGR_1.rd_port_urgent_en:13:1:=0x1
	 * PCFGR_1.rd_port_aging_en:12:1:=0x0
	 * PCFGR_1.read_reorder_bypass_en:11:1:=0x0
	 * PCFGR_1.rd_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x4b4);
	/*
	 * PCFGW_1.wr_port_pagematch_en:14:1:=0x1
	 * PCFGW_1.wr_port_urgent_en:13:1:=0x1
	 * PCFGW_1.wr_port_aging_en:12:1:=0x0
	 * PCFGW_1.wr_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x4b8);
	/* PCTRL_1.port_en:0:1:=0x1 */
	writel(0x00000001, DDRC_BASE + 0x540);
	/*
	 * PCFGQOS0_1.rqos_map_region2:24:8:=0x0
	 * PCFGQOS0_1.rqos_map_region1:20:4:=0x0
	 * PCFGQOS0_1.rqos_map_region0:16:4:=0x0
	 * PCFGQOS0_1.rqos_map_level2:8:8:=0x0
	 * PCFGQOS0_1.rqos_map_level1:0:8:=0x7
	 */
	writel(0x00000007, DDRC_BASE + 0x544);
	/*
	 * PCFGQOS1_1.rqos_map_timeoutr:16:16:=0x0
	 * PCFGQOS1_1.rqos_map_timeoutb:0:16:=0x6a
	 */
	writel(0x0000006a, DDRC_BASE + 0x548);
	/*
	 * PCFGWQOS0_1.wqos_map_region2:24:8:=0x0
	 * PCFGWQOS0_1.wqos_map_region1:20:4:=0x0
	 * PCFGWQOS0_1.wqos_map_region0:16:4:=0x0
	 * PCFGWQOS0_1.wqos_map_level2:8:8:=0xe
	 * PCFGWQOS0_1.wqos_map_level1:0:8:=0x7
	 */
	writel(0x00000e07, DDRC_BASE + 0x54c);
	/*
	 * PCFGWQOS1_1.wqos_map_timeout2:16:16:=0x1a8
	 * PCFGWQOS1_1.wqos_map_timeout1:0:16:=0x1a8
	 */
	writel(0x01a801a8, DDRC_BASE + 0x550);
	/*
	 * PCFGR_2.rdwr_ordered_en:16:1:=0x0
	 * PCFGR_2.rd_port_pagematch_en:14:1:=0x1
	 * PCFGR_2.rd_port_urgent_en:13:1:=0x1
	 * PCFGR_2.rd_port_aging_en:12:1:=0x0
	 * PCFGR_2.read_reorder_bypass_en:11:1:=0x0
	 * PCFGR_2.rd_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x564);
	/*
	 * PCFGW_2.wr_port_pagematch_en:14:1:=0x1
	 * PCFGW_2.wr_port_urgent_en:13:1:=0x1
	 * PCFGW_2.wr_port_aging_en:12:1:=0x0
	 * PCFGW_2.wr_port_priority:0:10:=0x0
	 */
	writel(0x00006000, DDRC_BASE + 0x568);
	/* PCTRL_2.port_en:0:1:=0x1 */
	writel(0x00000001, DDRC_BASE + 0x5f0);
	/*
	 * PCFGQOS0_2.rqos_map_region2:24:8:=0x0
	 * PCFGQOS0_2.rqos_map_region1:20:4:=0x0
	 * PCFGQOS0_2.rqos_map_region0:16:4:=0x0
	 * PCFGQOS0_2.rqos_map_level2:8:8:=0x0
	 * PCFGQOS0_2.rqos_map_level1:0:8:=0x7
	 */
	writel(0x00000007, DDRC_BASE + 0x5f4);
	/*
	 * PCFGQOS1_2.rqos_map_timeoutr:16:16:=0x0
	 * PCFGQOS1_2.rqos_map_timeoutb:0:16:=0x6a
	 */
	writel(0x0000006a, DDRC_BASE + 0x5f8);
	/*
	 * PCFGWQOS0_2.wqos_map_region2:24:8:=0x0
	 * PCFGWQOS0_2.wqos_map_region1:20:4:=0x0
	 * PCFGWQOS0_2.wqos_map_region0:16:4:=0x0
	 * PCFGWQOS0_2.wqos_map_level2:8:8:=0xe
	 * PCFGWQOS0_2.wqos_map_level1:0:8:=0x7
	 */
	writel(0x00000e07, DDRC_BASE + 0x5fc);
	/*
	 * PCFGWQOS1_2.wqos_map_timeout2:16:16:=0x1a8
	 * PCFGWQOS1_2.wqos_map_timeout1:0:16:=0x1a8
	 */
	writel(0x01a801a8, DDRC_BASE + 0x600);
}

void ctrl_init_high_patch(void)
{
	/* enable auto PD */
	writel(0x00000002, DDRC_BASE + 0x30);
	/* enable auto ctrl_upd */
	writel(0x00400018, DDRC_BASE + 0x1a0);
	/* enable clock gating */
	writel(0x00000000, DDR_TOP_BASE + 0x14);
	/* change xpi to multi DDR burst */
	writel(0x63746371, DDRC_BASE + 0xc);
	writel(0x08000000, DDRC_BASE + 0x44);
}

void ctrl_init_low_patch(void)
{
	/* disable auto PD/SR */
	writel(0x00000000, DDRC_BASE + 0x30);
	/* disable auto ctrl_upd */
	writel(0xc0400018, DDRC_BASE + 0x1a0);
	/* disable clock gating */
	writel(0x00000fff, DDR_TOP_BASE + 0x14);
	/* change xpi to single DDR burst */
	writel(0x63746371, DDRC_BASE + 0xc);
	writel(0x14000000, DDRC_BASE + 0x44);
}

/* RFSHTMG, DRAMTMG0 and DRAMTMG8 for 32 MiB (5) to 512 MiB (9) per device */
struct ddrc_size_timing {
	u32 rfshtmg;
	u32 dramtmg0;
	u32 dramtmg8;
};

#define DDRC_SIZE_MIN	5
#define DDRC_SIZE_MAX	9

static const struct ddrc_size_timing ddrc_size_timing_etron[] = {
	{ 0x00510019, 0x0b01160f, 0x00000502 },
	{ 0x00510023, 0x0b01160f, 0x00000503 },
	{ 0x0051002b, 0x0b0f160f, 0x00000503 },
	{ 0x00510041, 0x0b0f160f, 0x00000504 },
	{ 0x0051006e, 0x0b0f160f, 0x00000505 },
};

static const struct ddrc_size_timing ddrc_size_timing_esmt[] = {
	{ 0x00510019, 0x0b011610, 0x00000502 },
	{ 0x0051002c, 0x0a011610, 0x00000503 },
	{ 0x0051002b, 0x0b0f1610, 0x00000503 },
	{ 0x00510041, 0x0b0f1610, 0x00000504 },
	{ 0x0051006e, 0x0b0f1610, 0x00000505 },
};

static const u32 ddrc_addrmap_32m[DDRC_NUM_ADDRMAP] = {
	0x00001f1f, 0x003f0606, 0x00000000, 0x1f1f0000,
	0x00001f1f, 0x040f0404, 0x04040404, 0x00000404,
	0x00003f3f, 0x04040404, 0x04040404, 0x001f1f04,
};

static const u32 ddrc_addrmap_64m[DDRC_NUM_ADDRMAP] = {
	0x00001f1f, 0x003f0707, 0x00000000, 0x1f000000,
	0x00001f1f, 0x050f0505, 0x05050505, 0x00000505,
	0x00003f3f, 0x05050505, 0x05050505, 0x001f1f05,
};

static const u32 ddrc_addrmap_128m[DDRC_NUM_ADDRMAP] = {
	0x00001f1f, 0x00070707, 0x00000000, 0x1f000000,
	0x00001f1f, 0x060f0606, 0x06060606, 0x00000606,
	0x00003f3f, 0x06060606, 0x06060606, 0x001f1f06,
};

void ctrl_init_update_by_dram_size(u8 vendor, u8 dram_cap_in_mbyte)
{
	u32 mstr = readl(DDRC_BASE);
	u32 bus_width = FIELD_GET(GENMASK(13, 12), mstr);
	u32 dev_config = FIELD_GET(GENMASK(31, 30), mstr);
	u8 dram_cap_in_mbyte_per_dev = dram_cap_in_mbyte;
	const struct ddrc_size_timing *t;

	if (bus_width > 1 || dev_config > 2) {
		pr_err("DDR: unsupported MSTR %#x\n", mstr);
		return;
	}

	/* System capacity to x16 capacity, then to device capacity */
	dram_cap_in_mbyte_per_dev >>= 1 - bus_width;
	dram_cap_in_mbyte_per_dev >>= 2 - dev_config;

	if (dram_cap_in_mbyte_per_dev < DDRC_SIZE_MIN ||
	    dram_cap_in_mbyte_per_dev > DDRC_SIZE_MAX) {
		pr_err("DDR: no timing for %u MiB per device\n",
		       1 << dram_cap_in_mbyte_per_dev);
		return;
	}

	if (vendor == DDR_VENDOR_ETRON_512M_DDR2)
		t = &ddrc_size_timing_etron[dram_cap_in_mbyte_per_dev - DDRC_SIZE_MIN];
	else
		t = &ddrc_size_timing_esmt[dram_cap_in_mbyte_per_dev - DDRC_SIZE_MIN];

	writel(t->rfshtmg, DDRC_BASE + 0x64);
	writel(t->dramtmg0, DDRC_BASE + 0x100);
	writel(t->dramtmg8, DDRC_BASE + 0x120);

	if (dram_cap_in_mbyte_per_dev == 5)
		ddrc_set_addrmap(ddrc_addrmap_32m);
	else if (dram_cap_in_mbyte_per_dev == 6)
		ddrc_set_addrmap(ddrc_addrmap_64m);
	else
		ddrc_set_addrmap(ddrc_addrmap_128m);

	/* toggle refresh_update_level */
	writel(0x00000002, DDRC_BASE + 0x60);
	writel(0x00000000, DDRC_BASE + 0x60);
}
