// SPDX-License-Identifier: GPL-2.0-only
/*
 * Sophgo CV181x DDR3-1866 x16 controller settings
 *
 * Based on the Sophgo FSBL,
 * plat/cv181x/ddr/ddr_config/ddr3_1866_x16/ddrc_init.c.
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#include "ddr_sys.h"

void ddrc_init(u8 vendor)
{
	/*
	 * PATCH0.use_blk_ext}:0:2:=0x1
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
	 */
	writel(0x00000000, DDRC_BASE + 0x44);
	/*
	 * PATCH4.t_phyd_rden:16:6=0x0
	 * PATCH4.phyd_rd_clk_stop:23:1=0x0
	 * PATCH4.t_phyd_wren:24:6=0x0
	 * PATCH4.phyd_wr_clk_stop:31:1=0x0
	 */
	writel(0x999F0000, DDRC_BASE + 0x148);
	writel(0x81041401, DDRC_BASE);
	writel(0x00000000, DDRC_BASE + 0x30);
	writel(0x00930001, DDRC_BASE + 0x34);
	writel(0x00020000, DDRC_BASE + 0x38);
	writel(0x00201070, DDRC_BASE + 0x50);
	writel(0x00000000, DDRC_BASE + 0x60);
	writel(0x007100A4, DDRC_BASE + 0x64);
	writel(0x00000000, DDRC_BASE + 0xc0);
	writel(0x00000000, DDRC_BASE + 0xc4);
	writel(0x000100E5, DDRC_BASE + 0xd0);
	writel(0x006A0000, DDRC_BASE + 0xd4);
	writel(0x1F140040, DDRC_BASE + 0xdc);
	writel(0x00600000, DDRC_BASE + 0xe0);
	writel(0x000B03BF, DDRC_BASE + 0xe4);
	writel(0x0E111F10, DDRC_BASE + 0x100);
	if (vendor == DDR_VENDOR_ETRON_1G)
		writel(readl(DDRC_BASE + 0x100) + 0x3000000, DDRC_BASE + 0x100);
	writel(0x00030417, DDRC_BASE + 0x104);
	writel(0x0507060A, DDRC_BASE + 0x108);
	writel(0x00002007, DDRC_BASE + 0x10c);
	writel(0x07020307, DDRC_BASE + 0x110);
	writel(0x05050303, DDRC_BASE + 0x114);
	writel(0x00000907, DDRC_BASE + 0x120);
	writel(0x00000000, DDRC_BASE + 0x13c);
	writel(0xC0960026, DDRC_BASE + 0x180);
	writel(0x00000001, DDRC_BASE + 0x184);
	/* phyd related */
	/*
	 * DFITMG0.dfi_t_ctrl_delay:24:5:=0x4
	 * DFITMG0.dfi_rddata_use_dfi_phy_clk:23:1:=0x1
	 * DFITMG0.dfi_t_rddata_en:16:7:=0xa
	 * DFITMG0.dfi_wrdata_use_dfi_phy_clk:15:1:=0x1
	 * DFITMG0.dfi_tphy_wrdata:8:6:=0x3
	 * DFITMG0.dfi_tphy_wrlat:0:6:=0x5
	 */
	writel(0x048a8305, DDRC_BASE + 0x190);
	/*
	 * DFITMG1.dfi_t_cmd_lat:28:4:=0x0
	 * DFITMG1.dfi_t_parin_lat:24:2:=0x0
	 * DFITMG1.dfi_t_wrdata_delay:16:5:=0x7
	 * DFITMG1.dfi_t_dram_clk_disable:8:5:=0x2
	 * DFITMG1.dfi_t_dram_clk_enable:0:5:=0x2
	 */
	writel(0x00070202, DDRC_BASE + 0x194);
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
	writel(0xC0400018, DDRC_BASE + 0x1a0);
	writel(0x00FE00FF, DDRC_BASE + 0x1a4);
	writel(0x80000000, DDRC_BASE + 0x1a8);
	writel(0x000002C1, DDRC_BASE + 0x1b0);
	writel(0x00000001, DDRC_BASE + 0x1c0);
	writel(0x00000001, DDRC_BASE + 0x1c4);
	/* address map, auto gen. */
	writel(0x00001F1F, DDRC_BASE + 0x200);
	writel(0x00070707, DDRC_BASE + 0x204);
	writel(0x00000000, DDRC_BASE + 0x208);
	writel(0x1F000000, DDRC_BASE + 0x20c);
	writel(0x00001F1F, DDRC_BASE + 0x210);
	writel(0x060F0606, DDRC_BASE + 0x214);
	writel(0x06060606, DDRC_BASE + 0x218);
	writel(0x00000606, DDRC_BASE + 0x21c);
	writel(0x00003F3F, DDRC_BASE + 0x220);
	writel(0x06060606, DDRC_BASE + 0x224);
	writel(0x06060606, DDRC_BASE + 0x228);
	writel(0x001F1F06, DDRC_BASE + 0x22c);
	writel(0x08000610, DDRC_BASE + 0x240);
	writel(0x00000000, DDRC_BASE + 0x244);
	/*
	 * SCHED.opt_vprw_sch:31:1:=0x0
	 * SCHED.rdwr_idle_gap:24:7:=0x0
	 * SCHED.go2critical_hysteresis:16:8:=0x0
	 * SCHED.lpddr4_opt_act_timing:15:1:=0x0
	 * SCHED.lpr_num_entries:8:7:=0x1f
	 * SCHED.autopre_rmw:7:1:=0x1
	 * SCHED.dis_opt_ntt_by_pre:6:1:=0x0
	 * SCHED.dis_opt_ntt_by_act:5:1:=0x0
	 * SCHED.opt_wrcam_fill_level:4:1:=0x0
	 * SCHED.rdwr_switch_policy_sel:3:1:=0x0
	 * SCHED.pageclose:2:1:=0x1
	 * SCHED.prefer_write:1:1:=0x0
	 * SCHED.dis_opt_wrecc_collision_flush:0:1:=0x1
	 */
	writel(0x00003F85, DDRC_BASE + 0x250);
	writel(0x00000000, DDRC_BASE + 0x254);
	if (vendor == DDR_VENDOR_ETRON_1G)
	/*
	 * SCHED1.page_hit_limit_rd:28:3:=0x0
	 * SCHED1.page_hit_limit_wr:24:3:=0x0
	 * SCHED1.visible_window_limit_rd:20:3:=0x0
	 * SCHED1.visible_window_limit_wr:16:3:=0x0
	 * SCHED1.delay_switch_write:12:4:=0x0
	 * SCHED1.pageclose_timer:0:8:=0x0
	 */
		writel(0x40, DDRC_BASE + 0x254);
	/*
	 * PERFHPR1.hpr_xact_run_length:24:8:=0x20
	 * PERFHPR1.hpr_max_starve:0:16:=0x6a
	 */
	writel(0x100000F0, DDRC_BASE + 0x25c);
	/*
	 * PERFLPR1.lpr_xact_run_length:24:8:=0x20
	 * PERFLPR1.lpr_max_starve:0:16:=0x6a
	 */
	writel(0x100000F0, DDRC_BASE + 0x264);
	/*
	 * PERFWR1.w_xact_run_length:24:8:=0x20
	 * PERFWR1.w_max_starve:0:16:=0x1a8
	 */
	writel(0x100000F0, DDRC_BASE + 0x26c);
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
	/* enable auto PD/SR */
	writel(0x00000002, DDRC_BASE + 0x30);
	/* enable auto ctrl_upd */
	writel(0x00400018, DDRC_BASE + 0x1a0);
	/* enable clock gating */
	writel(0x00000000, DDR_TOP_BASE + 0x14);
}

void ctrl_init_low_patch(void)
{
	/* disable auto PD/SR */
	writel(0x00000000, DDRC_BASE + 0x30);
	/* disable auto ctrl_upd */
	writel(0xC0400018, DDRC_BASE + 0x1a0);
	/* disable clock gating */
	writel(0x00000fff, DDR_TOP_BASE + 0x14);
}

void ctrl_init_update_by_dram_size(u8 dram_cap_in_mbyte)
{
	u32 mstr = readl(DDRC_BASE);
	u8 dram_cap_in_mbyte_per_dev = dram_cap_in_mbyte;

	/* System capacity to x16 capacity, then to device capacity */
	dram_cap_in_mbyte_per_dev >>= 1 - FIELD_GET(GENMASK(13, 12), mstr);
	dram_cap_in_mbyte_per_dev >>= 2 - FIELD_GET(GENMASK(31, 30), mstr);

	switch (dram_cap_in_mbyte_per_dev) {
	case 6:
		writel(0x0071002A, DDRC_BASE + 0x64);
		writel(0x00000903, DDRC_BASE + 0x120);
		break;
	case 7:
		writel(0x00710034, DDRC_BASE + 0x64);
		writel(0x00000903, DDRC_BASE + 0x120);
		break;
	case 8:
		writel(0x0071004B, DDRC_BASE + 0x64);
		writel(0x00000904, DDRC_BASE + 0x120);
		break;
	case 9:
		writel(0x0071007A, DDRC_BASE + 0x64);
		writel(0x00000905, DDRC_BASE + 0x120);
		break;
	case 10:
		writel(0x007100A4, DDRC_BASE + 0x64);
		writel(0x00000907, DDRC_BASE + 0x120);
		break;
	}
	/* toggle refresh_update_level */
	writel(0x00000002, DDRC_BASE + 0x60);
	writel(0x00000000, DDRC_BASE + 0x60);
}
