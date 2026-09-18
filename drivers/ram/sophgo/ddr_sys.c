// SPDX-License-Identifier: GPL-2.0-only
/*
 * Sophgo CV181x DDR PHY training
 *
 * Based on the Sophgo FSBL, plat/cv181x/ddr/ddr_sys.c.
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#include "ddr_sys.h"

/* DDR PLL synthesizer for DDR_DATA_RATE, SSC itself stays disabled */
#define DDR_PLL_TAR_FREQ	(DDR_DATA_RATE >> 4)
#define DDR_PLL_SSC_SET		(752ULL * 67108864 / DDR_PLL_TAR_FREQ)
#define DDR_PLL_SSC_SPAN	(DDR_PLL_TAR_FREQ * 250 / 100)
#define DDR_PLL_SSC_STEP	(DDR_PLL_SSC_SET * 15 / (DDR_PLL_SSC_SPAN * 1000))

/* BIST controller */
#define BIST_CTRL		0x00
#define BIST_ADDR_START		0x10
#define BIST_ADDR_END		0x14
#define BIST_ADDR_STEP		0x18
#define BIST_SSO_PERIOD		0x24
#define BIST_CMD(n)		(0x40 + (n) * 4)
#define BIST_NUM_CMDS		6
#define BIST_STATUS		0x80
#define  BIST_STATUS_DONE	BIT(2)
#define  BIST_STATUS_FAIL	BIT(3)

#define BIST_OP_WRITE		1
#define BIST_OP_READ		2
#define BIST_OP_GOTO		3
#define BIST_PAT_0F		3
#define BIST_PAT_PRBS		5
#define BIST_PAT_SRAM		6
#define BIST_PAT_SSO_8X1_DM	7

/* op, SRAM start/stop, pattern and repeat count, no DQ/DM inversion or rotation */
#define BIST_CMD_VAL(op, start, stop, pat, repeat) \
	(((op) << 30) | ((start) << 21) | ((stop) << 12) | ((pat) << 9) | (repeat))
#define BIST_CMD_GOTO(loop)	((BIST_OP_GOTO << 30) | (loop))

#define BIST_SSO_FMIN		5
#define BIST_SSO_FMAX		15
#define BIST_SSO_SRAM_STOP						\
	(9 * (BIST_SSO_FMIN + BIST_SSO_FMAX) *				\
	 (BIST_SSO_FMAX - BIST_SSO_FMIN + 1) / 8 +			\
	 (BIST_SSO_FMAX - BIST_SSO_FMIN + 1))

/* DDR controller */
#define DDRC_PWRCTL			0x30
#define  DDRC_PWRCTL_SELFREF_SW		BIT(5)
#define  DDRC_PWRCTL_DFI_DRAM_CLK_DIS	BIT(3)
#define  DDRC_PWRCTL_POWERDOWN_EN	BIT(1)
#define  DDRC_PWRCTL_SELFREF_EN		BIT(0)
#define  DDRC_PWRCTL_TRAINING_MASK	(DDRC_PWRCTL_SELFREF_SW |	\
					 DDRC_PWRCTL_DFI_DRAM_CLK_DIS |	\
					 DDRC_PWRCTL_POWERDOWN_EN |	\
					 DDRC_PWRCTL_SELFREF_EN)
#define DDRC_RFSHCTL3			0x60
#define  DDRC_RFSHCTL3_DIS_AUTO_REFRESH	BIT(0)
#define DDRC_PCTRL(n)			(0x490 + 0xb0 * (n))
#define DDRC_NUM_PORTS			4
#define DDRC_PSTAT			0x3fc

/* PHY training status */
#define PHYD_TRAIN_DONE			0x3444
#define  PHYD_WRLVL_DONE		BIT(0)
#define  PHYD_RDGLVL_DONE		BIT(1)
#define  PHYD_RDLVL_DONE		BIT(2)
#define  PHYD_WDQLVL_DONE		BIT(3)

static void cvx16_bist_setup(const u32 *cmd, int count, u32 addr_step)
{
	int i;

	for (i = 0; i < BIST_NUM_CMDS; i++)
		writel(i < count ? cmd[i] : 0, DDR_BIST_BASE + BIST_CMD(i));

	writel(0x00000000, DDR_BIST_BASE + BIST_ADDR_START);
	writel(0x000fffff, DDR_BIST_BASE + BIST_ADDR_END);
	writel(addr_step, DDR_BIST_BASE + BIST_ADDR_STEP);
}

void cvx16_bist_wr_prbs_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, 511, BIST_PAT_PRBS, 0),
		BIST_CMD_VAL(BIST_OP_READ, 0, 511, BIST_PAT_PRBS, 0),
	};

	/* BIST clock enable */
	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

void cvx16_bist_wr_sram_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 15),
		BIST_CMD_VAL(BIST_OP_READ, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 15),
		0,
		BIST_CMD_GOTO(1),
	};

	/* BIST clock enable, axi_len 8 */
	writel(0x000c000c, DDR_BIST_BASE + BIST_CTRL);
	writel((BIST_SSO_FMAX << 8) | BIST_SSO_FMIN, DDR_BIST_BASE + BIST_SSO_PERIOD);
	/* 2 KiB AXI address step */
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 2048 / 4 / 16);
}

static void cvx16_bist_wrlvl_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, 0, BIST_PAT_PRBS, 0),
	};

	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

static void cvx16_bist_rdglvl_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_READ, 0, 3, BIST_PAT_PRBS, 0),
	};

	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

/* Continuous PRBS and SRAM write/read */
static void cvx16_bist_rdlvl_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, 511, BIST_PAT_PRBS, 2),
		BIST_CMD_VAL(BIST_OP_READ, 0, 511, BIST_PAT_PRBS, 2),
		BIST_CMD_VAL(BIST_OP_WRITE, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 2),
		BIST_CMD_VAL(BIST_OP_READ, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 2),
		BIST_CMD_GOTO(1),
	};

	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	writel((BIST_SSO_FMAX << 8) | BIST_SSO_FMIN, DDR_BIST_BASE + BIST_SSO_PERIOD);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

/* PRBS and SRAM write/read */
static void cvx16_bist_wdqlvl_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, 511, BIST_PAT_PRBS, 0),
		BIST_CMD_VAL(BIST_OP_READ, 0, 511, BIST_PAT_PRBS, 0),
		BIST_CMD_VAL(BIST_OP_WRITE, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 0),
		BIST_CMD_VAL(BIST_OP_READ, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SRAM, 0),
	};

	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	writel((BIST_SSO_FMAX << 8) | BIST_SSO_FMIN, DDR_BIST_BASE + BIST_SSO_PERIOD);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

static void cvx16_bist_wdmlvl_init(void)
{
	static const u32 cmd[] = {
		BIST_CMD_VAL(BIST_OP_WRITE, 0, BIST_SSO_SRAM_STOP, BIST_PAT_0F, 0),
		BIST_CMD_VAL(BIST_OP_WRITE, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SSO_8X1_DM, 0),
		BIST_CMD_VAL(BIST_OP_READ, 0, BIST_SSO_SRAM_STOP, BIST_PAT_SSO_8X1_DM, 0),
	};

	writel(0x00060006, DDR_BIST_BASE + BIST_CTRL);
	writel((BIST_SSO_FMAX << 8) | BIST_SSO_FMIN, DDR_BIST_BASE + BIST_SSO_PERIOD);
	cvx16_bist_setup(cmd, ARRAY_SIZE(cmd), 4);
}

/* Run the programmed BIST, returns 0 if it passed */
int cvx16_bist_start_check(void)
{
	u32 status;
	int ret;

	/* BIST enable */
	writel(0x00030003, DDR_BIST_BASE + BIST_CTRL);

	ret = ddr_readl_poll(DDR_BIST_BASE + BIST_STATUS, status,
			     status & BIST_STATUS_DONE);
	if (!ret && (status & BIST_STATUS_FAIL))
		ret = -EIO;

	/* BIST disable */
	writel(0x00050000, DDR_BIST_BASE + BIST_CTRL);

	return ret;
}

static void cvx16_rdvld_train(void)
{
	u32 byte0_vld, rdvld_offset;
	int i;

	cvx16_bist_wr_prbs_init();

	byte0_vld = readl(PHYD_BASE + 0xb14);
	rdvld_offset = FIELD_GET(GENMASK(3, 0), readl(PHYD_BASE + 0x94));

	/*
	 * Lower the read valid window until the BIST fails, then step back.
	 * Byte 1 is written with the byte 0 value, as the vendor code does.
	 */
	for (i = 9; i > 1; i--) {
		byte0_vld = u32_replace_bits(byte0_vld, i, GENMASK(20, 16));
		writel(byte0_vld, PHYD_BASE + 0xb14);
		writel(byte0_vld, PHYD_BASE + 0xb44);
		if (cvx16_bist_start_check()) {
			i += 1 + rdvld_offset;
			byte0_vld = u32_replace_bits(byte0_vld, i, GENMASK(20, 16));
			writel(byte0_vld, PHYD_BASE + 0xb14);
			writel(byte0_vld, PHYD_BASE + 0xb44);
			break;
		}
	}
}

/* Mode register write through the DDR controller */
static void cvx16_synp_mrw(u32 addr, u32 data)
{
	u32 zqctl0, rddata;

	/* Disable auto ZQ calibration during the write */
	zqctl0 = readl(DDRC_BASE + 0x180);
	if (!(zqctl0 & BIT(31)))
		writel(zqctl0 | BIT(31), DDRC_BASE + 0x180);

	/* MRSTAT.mr_wr_busy */
	ddr_readl_poll(DDRC_BASE + 0x18, rddata, !(rddata & BIT(0)));

	/* MRCTRL0: write to rank 0 at mr_addr, then MRCTRL1: mr_data */
	writel(FIELD_PREP(GENMASK(5, 4), 1) | FIELD_PREP(GENMASK(15, 12), addr),
	       DDRC_BASE + 0x10);
	writel(data, DDRC_BASE + 0x14);
	/* MRCTRL0.mr_wr */
	setbits_le32(DDRC_BASE + 0x10, BIT(31));

	if (!(zqctl0 & BIT(31)))
		clrbits_le32(DDRC_BASE + 0x180, BIT(31));
}

static void cvx16_ddr_pll_ssc_set(void)
{
	u32 rddata;

	writel(DDR_PLL_SSC_SET, TOP_BASE + 0x2954);
	writel(FIELD_GET(GENMASK(15, 0), DDR_PLL_SSC_SPAN), TOP_BASE + 0x2958);
	writel(FIELD_GET(GENMASK(23, 0), DDR_PLL_SSC_STEP), TOP_BASE + 0x295c);

	rddata = readl(TOP_BASE + 0x2950);
	/* Toggle SSC_SW_UP, disable SSC and bypass, set EXTPULSE */
	rddata ^= BIT(0);
	rddata &= ~(BIT(1) | GENMASK(3, 2) | BIT(4) | BIT(6));
	rddata |= BIT(5);
	writel(rddata, TOP_BASE + 0x2950);
}

static void cvx16_clk_normal(void)
{
	/* DDRPLL_SEL_LOW_SPEED = 0, DDRPLL_MAS_DIV_OUT_SEL = 0 */
	clrbits_le32(PHYD_APB_BASE + 0xc, BIT(13) | BIT(14));
	cvx16_ddr_pll_ssc_set();
}

static void cvx16_clk_div2(void)
{
	/* DDRPLL_MAS_DIV_OUT_SEL = 1 */
	setbits_le32(PHYD_APB_BASE + 0xc, BIT(14));
}

static void cvx16_clk_div40(void)
{
	/* DDRPLL_SEL_LOW_SPEED = 1 */
	setbits_le32(PHYD_APB_BASE + 0xc, BIT(13));
}

static void cvx16_chg_pll_freq(void)
{
	u32 rddata, cur_speed, next_speed;

	/* Assert RESETZ_DIV, RESETZ_DQS and DDRPLL_MAS_RSTZ_DIV */
	writel(0, PHYD_APB_BASE + 0x4);
	writel(0, PHYD_APB_BASE + 0x8);
	clrbits_le32(PHYD_APB_BASE + 0xc, BIT(7));

	/* [0] EN_PLL_SPEED_CHG, [5:4] CUR_PLL_SPEED, [9:8] NEXT_PLL_SPEED */
	rddata = readl(PHYD_APB_BASE + 0x4c);
	cur_speed = FIELD_GET(GENMASK(5, 4), rddata);
	next_speed = FIELD_GET(GENMASK(9, 8), rddata);

	if ((rddata & BIT(0)) && next_speed != 3) {
		rddata = u32_replace_bits(rddata, next_speed, GENMASK(5, 4));
		rddata = u32_replace_bits(rddata, cur_speed, GENMASK(9, 8));
		writel(rddata, PHYD_APB_BASE + 0x4c);

		if (next_speed == 0)
			cvx16_clk_div40();
		else if (next_speed == 1)
			cvx16_clk_div2();
		else
			cvx16_clk_normal();
	}

	/* De-assert RESETZ_DIV, DDRPLL_MAS_RSTZ_DIV and RESETZ_DQS */
	writel(1, PHYD_APB_BASE + 0x4);
	setbits_le32(PHYD_APB_BASE + 0xc, BIT(7));
	writel(1, PHYD_APB_BASE + 0x8);

	/* DDRPLL lock */
	ddr_readl_poll(PHYD_APB_BASE + 0x10, rddata, rddata & BIT(15));
}

static void cvx16_dll_cal(void)
{
	u32 rddata;

	/* param_phyd_dll_rx_start_cal [1], param_phyd_dll_tx_start_cal [17] */
	clrbits_le32(PHYD_BASE + 0x40, BIT(1) | BIT(17));

	/* Only calibrate at high speed */
	if (FIELD_GET(GENMASK(5, 4), readl(PHYD_APB_BASE + 0x4c))) {
		setbits_le32(PHYD_BASE + 0x40, BIT(1) | BIT(17));
		ddr_readl_poll(PHYD_BASE + 0x3014, rddata, rddata & BIT(16));
	}
}

static void cvx16_set_dfi_init_complete(void)
{
	udelay(20);
	writel(0x00000010, PHYD_BASE + 0x120);
	/* param_phyd_clkctrl_init_complete */
	writel(0x00000001, PHYD_BASE + 0x118);
}

void cvx16_ddr_phy_power_on_seq1(void)
{
	/* TX_CA_PD_CKE0 and TX_CA_PD_RESETZ, then all CA PD = 0 */
	clrbits_le32(PHYD_APB_BASE + 0x40, BIT(24) | BIT(30));
	writel(0, PHYD_APB_BASE + 0x40);

	/* TX_SEL_GPIO = 1, DQ PD = 0, TX_SEL_GPIO = 0 */
	setbits_le32(PHYD_APB_BASE + 0x1c, BIT(7));
	writel(0, PHYD_APB_BASE);
	clrbits_le32(PHYD_APB_BASE + 0x1c, BIT(7));
}

static void cvx16_ddr_phy_power_on_seq2(void)
{
	cvx16_chg_pll_freq();

	/* param_phyd_sel_cke_oenz, param_phyd_tx_ca_*_oenz */
	clrbits_le32(PHYD_BASE + 0x154, BIT(0));
	writel(0, PHYD_BASE + 0x130);

	cvx16_dll_cal();

	/* CA PD = 0, BYTE PD = 0 */
	writel(0x80000000, PHYD_APB_BASE + 0x40);
	writel(0, PHYD_APB_BASE);
}

void cvx16_ddr_phy_power_on_seq3(void)
{
	/* param_phyd_sel_cke_oenz, param_phyd_tx_ca_*_oenz */
	clrbits_le32(PHYD_BASE + 0x154, BIT(0));
	writel(0, PHYD_BASE + 0x130);

	/* Gate the extended OENZ dline clock of both bytes to save power */
	setbits_le32(PHYD_BASE + 0x204, BIT(18));
	setbits_le32(PHYD_BASE + 0x224, BIT(18));
}

/* Handle the PHY dfi_init_start request */
void cvx16_dfi_init_start_isr(void)
{
	/* param_phyd_clkctrl_init_complete */
	writel(0, PHYD_BASE + 0x118);
	cvx16_ddr_phy_power_on_seq2();
	cvx16_set_dfi_init_complete();
}

void cvx16_wait_for_dfi_init_complete(void)
{
	u32 rddata;

	/* DFISTAT.dfi_init_complete */
	ddr_readl_poll(DDRC_BASE + 0x1bc, rddata, rddata & BIT(0));
	writel(0, DDRC_BASE + 0x320);
	clrsetbits_le32(DDRC_BASE + 0x1b0, GENMASK(5, 0), 5);
	writel(1, DDRC_BASE + 0x320);
}

void cvx16_polling_dfi_init_start(void)
{
	u32 rddata;

	ddr_readl_poll(PHYD_BASE + 0x3028, rddata, rddata & BIT(8));
}

void cvx16_polling_synp_normal_mode(void)
{
	u32 rddata;

	/* STAT.operating_mode */
	ddr_readl_poll(DDRC_BASE + 0x4, rddata, FIELD_GET(GENMASK(2, 0), rddata) == 1);
}

static void cvx16_dfi_ca_park_prbs(bool enable)
{
	u32 rddata;

	/* param_phyd_sw_dfi_phyupd_req */
	writel(0x00000001, PHYD_BASE + 0x174);
	/* dfi_phyupd_req and dfi_phyupd_ack */
	ddr_readl_poll(PHYD_BASE + 0x3030, rddata,
		       FIELD_GET(GENMASK(9, 8), rddata) == 3);

	writel(enable ? 0x1b : 0, DDR_TOP_BASE);
	writel(0x01, DDR_TOP_BASE + 0x4);
	writel(0x1ffffcb, DDR_TOP_BASE + 0x8);
	writel(0x3fffffff, DDR_TOP_BASE + 0xc);

	/* param_phyd_sw_dfi_phyupd_req_clr */
	writel(0x00000010, PHYD_BASE + 0x174);
}

static void cvx16_clk_gating_disable(void)
{
	/* TOP_REG_CG_EN_* */
	writel(0x000012f5, PHYD_APB_BASE + 0x44);
	/* PHYD_SHIFT_GATING_EN */
	writel(0x00000000, PHYD_BASE + 0xf4);
	/* phyd_stop_clk */
	clrbits_le32(DDRC_BASE + 0x30, BIT(9));
	/* DFI read/write clock gating */
	clrbits_le32(DDRC_BASE + 0x148, BIT(23) | BIT(31));
}

void cvx16_clk_gating_enable(void)
{
	/* TOP_REG_CG_EN_* */
	writel(0x00002c81, PHYD_APB_BASE + 0x44);
	clrsetbits_le32(DDRC_BASE + 0x190, GENMASK(28, 24), FIELD_PREP(GENMASK(28, 24), 6));
	/* PHYD_SHIFT_GATING_EN */
	writel(0x00030033, PHYD_BASE + 0xf4);
	/* phyd_stop_clk */
	setbits_le32(DDRC_BASE + 0x30, BIT(9));
	/* DFI read/write clock gating */
	setbits_le32(DDRC_BASE + 0x148, BIT(23) | BIT(31));
}

/*
 * Quiesce the controller before a training step: disable all AXI ports but
 * port 0, low power modes and clock gating. Returns the PWRCTL bits to restore.
 */
static u32 cvx16_training_enter(void)
{
	u32 rddata, pwrctl;
	int i;

	for (i = 1; i < DDRC_NUM_PORTS; i++)
		writel(0, DDRC_BASE + DDRC_PCTRL(i));

	/* PSTAT.rd_port_busy_n and PSTAT.wr_port_busy_n */
	ddr_readl_poll(DDRC_BASE + DDRC_PSTAT, rddata, rddata == 0);

	/* PWRCTL.deeppowerdown_en must stay 0 for DDR3 */
	pwrctl = readl(DDRC_BASE + DDRC_PWRCTL);
	writel(pwrctl & ~DDRC_PWRCTL_TRAINING_MASK, DDRC_BASE + DDRC_PWRCTL);
	cvx16_clk_gating_disable();

	return pwrctl & DDRC_PWRCTL_TRAINING_MASK;
}

static void cvx16_training_exit(u32 pwrctl)
{
	int i;

	clrsetbits_le32(DDRC_BASE + DDRC_PWRCTL, DDRC_PWRCTL_TRAINING_MASK, pwrctl);

	for (i = 1; i < DDRC_NUM_PORTS; i++)
		writel(1, DDRC_BASE + DDRC_PCTRL(i));

	cvx16_clk_gating_enable();
}

static void cvx16_mpr_mode(bool enable)
{
	u32 mr3 = readl(DDRC_BASE + 0xe0);

	if (enable) {
		setbits_le32(DDRC_BASE + DDRC_RFSHCTL3, DDRC_RFSHCTL3_DIS_AUTO_REFRESH);
		/* MR3: dataflow from MPR */
		cvx16_synp_mrw(0x3, FIELD_GET(GENMASK(15, 0), mr3 | BIT(2)));
	} else {
		/* MR3: normal operation */
		cvx16_synp_mrw(0x3, FIELD_GET(GENMASK(15, 0), mr3 & ~BIT(2)));
		clrbits_le32(DDRC_BASE + DDRC_RFSHCTL3, DDRC_RFSHCTL3_DIS_AUTO_REFRESH);
	}
}

/* Needs ctrl_init_low_patch() first, as all the training steps */
void cvx16_wrlvl_req(void)
{
	u32 rddata, pwrctl, rtt_wr, rtt_nom = 0;
	bool wr_odt_en;

	/* Write leveling response only on DQ0 */
	writel(0x00fe0000, PHYD_BASE + 0x5c);

	pwrctl = cvx16_training_enter();

	wr_odt_en = readl(DDRC_BASE + 0x244) & BIT(0);
	cvx16_bist_wrlvl_init();

	if (wr_odt_en) {
		rddata = readl(DDRC_BASE + 0xe0);
		rtt_wr = FIELD_GET(GENMASK(26, 25), rddata);
		if (rtt_wr) {
			/* Disable rtt_wr in MR2, move it to rtt_nom */
			rddata = u32_replace_bits(rddata, 0, GENMASK(26, 25));
			cvx16_synp_mrw(0x2, FIELD_GET(GENMASK(31, 16), rddata));
			rtt_nom = readl(DDRC_BASE + 0xdc) & ~BIT(9);
			rtt_nom = u32_replace_bits(rtt_nom, FIELD_GET(BIT(1), rtt_wr), BIT(6));
			rtt_nom = u32_replace_bits(rtt_nom, FIELD_GET(BIT(0), rtt_wr), BIT(2));
		}
	} else {
		/* rtt_nom = 120 Ohm */
		rtt_nom = readl(DDRC_BASE + 0xdc);
		rtt_nom &= ~(BIT(9) | BIT(2));
		rtt_nom |= BIT(6);
		cvx16_synp_mrw(0x1, FIELD_GET(GENMASK(15, 0), rtt_nom));
	}
	/* MR1: write leveling enable */
	cvx16_synp_mrw(0x1, FIELD_GET(GENMASK(15, 0), rtt_nom | BIT(7)));

	/* param_phyd_dfi_wrlvl_req, param_phyd_dfi_wrlvl_odt_en */
	rddata = readl(PHYD_BASE + 0x180);
	rddata |= BIT(0);
	rddata = u32_replace_bits(rddata, wr_odt_en, BIT(4));
	writel(rddata, PHYD_BASE + 0x180);

	ddr_readl_poll(PHYD_BASE + PHYD_TRAIN_DONE, rddata, rddata & PHYD_WRLVL_DONE);
	/* BIST clock disable */
	writel(0x00040000, DDR_BIST_BASE + BIST_CTRL);

	clrbits_le32(DDRC_BASE + DDRC_RFSHCTL3, DDRC_RFSHCTL3_DIS_AUTO_REFRESH);

	/* Restore MR1 and MR2 */
	cvx16_synp_mrw(0x1, FIELD_GET(GENMASK(15, 0), readl(DDRC_BASE + 0xdc)));
	cvx16_synp_mrw(0x2, FIELD_GET(GENMASK(31, 16), readl(DDRC_BASE + 0xe0)));

	cvx16_training_exit(pwrctl);
}

void cvx16_rdglvl_req(void)
{
	u32 rddata, pwrctl;
	bool mpr_mode;

	pwrctl = cvx16_training_enter();

	mpr_mode = readl(PHYD_BASE + 0x184) & BIT(4);
	if (mpr_mode)
		cvx16_mpr_mode(true);

	cvx16_bist_rdglvl_init();
	/* param_phyd_dfi_rdglvl_req */
	setbits_le32(PHYD_BASE + 0x184, BIT(0));

	ddr_readl_poll(PHYD_BASE + PHYD_TRAIN_DONE, rddata, rddata & PHYD_RDGLVL_DONE);
	/* BIST clock disable */
	writel(0x00040000, DDR_BIST_BASE + BIST_CTRL);

	if (mpr_mode)
		cvx16_mpr_mode(false);

	cvx16_training_exit(pwrctl);
}

void cvx16_rdlvl_req(void)
{
	u32 rddata, pwrctl;
	bool mpr_mode, vref_training_en;

	pwrctl = cvx16_training_enter();
	cvx16_dfi_ca_park_prbs(true);

	/* param_phyd_pirdlvl_deskew_start = 0x20, param_phyd_pirdlvl_deskew_end = 0x1f */
	rddata = readl(PHYD_BASE + 0x80);
	rddata = u32_replace_bits(rddata, 0x20, GENMASK(22, 16));
	rddata = u32_replace_bits(rddata, 0x1f, GENMASK(30, 24));
	writel(rddata, PHYD_BASE + 0x80);

	/*
	 * Save param_phyd_pirdlvl_vref_training_en [2], then clear it together
	 * with param_phyd_pirdlvl_rx_init_deskew_en [1] and
	 * param_phyd_pirdlvl_rdvld_training_en [3]
	 */
	rddata = readl(PHYD_BASE + 0x8c);
	vref_training_en = rddata & BIT(2);
	writel(rddata & ~GENMASK(3, 1), PHYD_BASE + 0x8c);

	mpr_mode = readl(PHYD_BASE + 0x188) & BIT(4);
	if (mpr_mode)
		cvx16_mpr_mode(true);

	cvx16_bist_rdlvl_init();
	/* param_phyd_dfi_rdlvl_req */
	setbits_le32(PHYD_BASE + 0x188, BIT(0));
	ddr_readl_poll(PHYD_BASE + PHYD_TRAIN_DONE, rddata, rddata & PHYD_RDLVL_DONE);

	if (vref_training_en) {
		/* Final training, keep the RX trigger level */
		clrbits_le32(PHYD_BASE + 0x8c, BIT(2));
		setbits_le32(PHYD_BASE + 0x188, BIT(0));
		ddr_readl_poll(PHYD_BASE + PHYD_TRAIN_DONE, rddata, rddata & PHYD_RDLVL_DONE);
		setbits_le32(PHYD_BASE + 0x8c, BIT(2));
	}

	if (mpr_mode)
		cvx16_mpr_mode(false);

	cvx16_rdvld_train();

	/* BIST clock disable */
	writel(0x00040000, DDR_BIST_BASE + BIST_CTRL);
	cvx16_dfi_ca_park_prbs(false);
	cvx16_training_exit(pwrctl);
}

/**
 * cvx16_wdqlvl_req() - Write DQ/DM leveling with BIST data
 * @lvl_mode: WDQLVL_DM, WDQLVL_DQ or WDQLVL_DQ_DM
 */
void cvx16_wdqlvl_req(u32 lvl_mode)
{
	u32 rddata, pwrctl;

	pwrctl = cvx16_training_enter();
	cvx16_dfi_ca_park_prbs(true);

	/* param_phyd_piwdqlvl_dq_mode [12], param_phyd_piwdqlvl_dm_mode [13] */
	rddata = readl(PHYD_BASE + 0xbc);
	rddata = u32_replace_bits(rddata, lvl_mode != WDQLVL_DM, BIT(12));
	rddata = u32_replace_bits(rddata, lvl_mode != WDQLVL_DQ, BIT(13));
	writel(rddata, PHYD_BASE + 0xbc);

	if (lvl_mode == WDQLVL_DM) {
		setbits_le32(DDRC_BASE + 0xc, BIT(7));
		cvx16_bist_wdmlvl_init();
	} else {
		cvx16_bist_wdqlvl_init();
	}

	/*
	 * param_phyd_dfi_wdqlvl_req [0], param_phyd_dfi_wdqlvl_bist_data_en [4],
	 * param_phyd_dfi_wdqlvl_vref_train_en [10]
	 */
	rddata = readl(PHYD_BASE + 0x18c);
	rddata |= BIT(0) | BIT(4);
	rddata = u32_replace_bits(rddata, lvl_mode != WDQLVL_DM, BIT(10));
	writel(rddata, PHYD_BASE + 0x18c);

	ddr_readl_poll(PHYD_BASE + PHYD_TRAIN_DONE, rddata, rddata & PHYD_WDQLVL_DONE);

	clrbits_le32(DDRC_BASE + 0xc, BIT(7));
	/* BIST clock disable */
	writel(0x00040000, DDR_BIST_BASE + BIST_CTRL);
	cvx16_dfi_ca_park_prbs(false);
	cvx16_training_exit(pwrctl);
}

void cvx16_set_dfi_init_start(void)
{
	/* Trigger dfi_init_start from the controller, the PHY is ready */
	writel(0, DDRC_BASE + 0x320);
	setbits_le32(DDRC_BASE + 0x1b0, BIT(5));
	writel(1, DDRC_BASE + 0x320);
}

void cvx16_pll_init(void)
{
	u32 rddata;

	/* TX_VREF_PD */
	writel(0x00000000, PHYD_APB_BASE + 0x28);
	/* ZQ_240 option */
	writel(0x00080001, PHYD_APB_BASE + 0x54);
	/* TOP_REG_TX_DDR3_GPO_IN = 1 */
	writel(0x01010808, PHYD_APB_BASE + 0x58);

	cvx16_ddr_pll_ssc_set();

	/*
	 * DDRPLL: EN_DLLCLK, EN_LCKDET, ICTRL = 1, SEL_4BIT, SEL_MODE = 1,
	 * everything else cleared
	 */
	clrsetbits_le32(PHYD_APB_BASE + 0xc, GENMASK(15, 0), 0x030b);
	/* TOP_REG_DDRPLL_TEST */
	clrbits_le32(PHYD_APB_BASE + 0x10, GENMASK(7, 0));
	/* TOP_REG_RESETZ_DIV, TOP_REG_DDRPLL_MAS_RSTZ_DIV */
	writel(0x1, PHYD_APB_BASE + 0x4);
	setbits_le32(PHYD_APB_BASE + 0xc, BIT(7));

	ddr_readl_poll(PHYD_APB_BASE + 0x10, rddata, rddata & BIT(15));
}

/*
 * Write PRBS at address 0 and its inverse at increasing powers of two until
 * the pattern at address 0 gets corrupted by the aliased write.
 */
void ctrl_init_detect_dram_size(u8 *dram_cap_in_mbyte)
{
	u32 cmd[BIST_NUM_CMDS] = { 0 };
	u8 cap_in_mbyte = 4;
	u32 rddata;
	int i;

	/* axsize = 3, axlen = 4, cgen */
	writel(0x000e0006, DDR_BIST_BASE + BIST_CTRL);

	writel(0x00000000, DDR_BIST_BASE + BIST_ADDR_START);
	writel(0xffffffff, DDR_BIST_BASE + BIST_ADDR_END);
	writel(0x00000004, DDR_BIST_BASE + BIST_ADDR_STEP);

	/* Write 16 UI of PRBS at address 0 as background */
	cmd[0] = BIST_CMD_VAL(BIST_OP_WRITE, 0, 3, BIST_PAT_PRBS, 0);
	for (i = 0; i < BIST_NUM_CMDS; i++)
		writel(cmd[i], DDR_BIST_BASE + BIST_CMD(i));

	writel(0x00010001, DDR_BIST_BASE + BIST_CTRL);
	ddr_readl_poll(DDR_BIST_BASE + BIST_STATUS, rddata, rddata & BIST_STATUS_DONE);
	writel(0x00010000, DDR_BIST_BASE + BIST_CTRL);

	do {
		cap_in_mbyte++;

		/* Write 16 UI of ~PRBS at 1 << cap_in_mbyte MiB */
		writel(1 << (cap_in_mbyte + 20 - 4), DDR_BIST_BASE + BIST_ADDR_START);
		cmd[0] = BIST_CMD_VAL(BIST_OP_WRITE, 0, 3, BIST_PAT_PRBS, 0) | BIT(8);
		for (i = 0; i < BIST_NUM_CMDS; i++)
			writel(cmd[i], DDR_BIST_BASE + BIST_CMD(i));

		writel(0x00010001, DDR_BIST_BASE + BIST_CTRL);
		ddr_readl_poll(DDR_BIST_BASE + BIST_STATUS, rddata, rddata & BIST_STATUS_DONE);
		writel(0x00010000, DDR_BIST_BASE + BIST_CTRL);

		/* Check the PRBS at address 0 */
		writel(0x00000000, DDR_BIST_BASE + BIST_ADDR_START);
		cmd[0] = BIST_CMD_VAL(BIST_OP_READ, 0, 3, BIST_PAT_PRBS, 0);
		for (i = 0; i < BIST_NUM_CMDS; i++)
			writel(cmd[i], DDR_BIST_BASE + BIST_CMD(i));

		writel(0x00010001, DDR_BIST_BASE + BIST_CTRL);
		ddr_readl_poll(DDR_BIST_BASE + BIST_STATUS, rddata, rddata & BIST_STATUS_DONE);
		writel(0x00010000, DDR_BIST_BASE + BIST_CTRL);
	} while (!(rddata & BIST_STATUS_FAIL) && cap_in_mbyte < 15);

	*dram_cap_in_mbyte = cap_in_mbyte;
	writel(cap_in_mbyte, PHYD_BASE + 0x208);

	/* cgen disable */
	writel(0x00040000, DDR_BIST_BASE + BIST_CTRL);
}
