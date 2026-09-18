// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * PLL setup based on the Sophgo FSBL, plat/cv181x/platform.c.
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

#define CLKGEN_BASE			0x03002000UL
#define CLK_SEL_0			0x020
#define  CLK_SEL_0_C906_1		BIT(24)
#define  CLK_SEL_0_C906_0		BIT(23)
#define CLK_BYP_0			0x030
#define CLK_BYP_1			0x034
#define PLL_G2_CTRL			0x800
#define  PLL_G2_CTRL_PD			0x00011111
#define MIPIMPLL_CSR			0x808
#define APLL0_CSR			0x80c
#define PLL_G2_SSC_SYN_CTRL		0x840
#define APLL_SSC_SYN_CTRL		0x850
#define  SSC_SYN_CTRL_BYPASS		BIT(4)
#define  SSC_SYN_CTRL_SW_UP		BIT(0)
#define APLL_SSC_SYN_SET		0x854
#define MIPIMPLL_D3_PD			0x8a0
#define  MIPIMPLL_D3_PD_PD		BIT(2)
#define CAM0PLL_D2_PD			0x8ac
#define  CAM0PLL_D2_PD_PD		(BIT(2) | BIT(1))
#define MPLL_CSR			0x908
#define TPLL_CSR			0x90c

/*
 * Divider register: [20:16] divider factor, [9:8] clock source, [3] take the
 * divider factor from the register, [0] de-assert the divider reset.
 */
#define CLK_DIV(div, src)		(((div) << 16) | ((src) << 8) | BIT(3) | BIT(0))
#define clkgen_reg(off)			((void __iomem *)(CLKGEN_BASE + (off)))

struct cv18xx_clk_div {
	u16 off;
	u32 val;
};

static const struct cv18xx_clk_div cv18xx_clk_divs[] = {
	{ 0x130, CLK_DIV(1, 3) },	/* clk_c906_0 = MPLL(850) / 1 */
	{ 0x138, CLK_DIV(2, 2) },	/* clk_c906_1 = DISPPLL(1188) / 2 */
	{ 0x054, CLK_DIV(3, 3) },	/* clk_tpu = FPLL(1500) / 3 */
	{ 0x048, CLK_DIV(3, 0) },	/* clk_cpu_axi0 = FPLL(1500) / 3 */
	{ 0x064, CLK_DIV(4, 0) },	/* clk_emmc = FPLL(1500) / 4 */
	{ 0x088, CLK_DIV(8, 0) },	/* clk_spi_nand = FPLL(1500) / 8 */
	{ 0x098, CLK_DIV(18, 0) },	/* clk_sdma_aud0 = APLL(442.368) / 18 */
	{ 0x120, CLK_DIV(15, 0) },	/* clk_pwm_src = FPLL(1500) / 15 */
	{ 0x0a8, CLK_DIV(1, 0) },	/* clk_cam0_200 = XTAL(25) / 1 */
	{ 0x0c8, CLK_DIV(3, 0) },	/* clk_axi_vip = MIPIPLL(900) / 3 */
	{ 0x110, CLK_DIV(2, 2) },	/* clk_src_vip_sys_2 = DISPPLL(1188) / 2 */
	{ 0x144, CLK_DIV(3, 2) },	/* clk_src_vip_sys_4 = DISPPLL(1188) / 3 */
	{ 0x0e4, CLK_DIV(3, 2) },	/* clk_axi_video_codec = CAM1PLL(1080) / 3 */
	{ 0x0ec, CLK_DIV(3, 0) },	/* clk_vc_src0 = DISPPLL(1188) / 3 */
	{ 0x0d0, CLK_DIV(6, 2) },	/* clk_src_vip_sys_0 = DISPPLL(1188) / 6 */
	{ 0x0d8, CLK_DIV(3, 0) },	/* clk_src_vip_sys_1 = MIPIPLL(900) / 3 */
	{ 0x140, CLK_DIV(3, 0) },	/* clk_src_vip_sys_3 = MIPIPLL(900) / 3 */
	{ 0x0b8, CLK_DIV(5, 0) },	/* clk_axi4 = FPLL(1500) / 5 */
	{ 0x12c, CLK_DIV(5, 0) },	/* clk_src_rtc_sys_0 = FPLL(1500) / 5 */
};

/* APLL, DISPPLL, CAM0PLL and CAM1PLL synthesizers and dividers */
static const u32 cv18xx_pll_syn_set[] = {
	614440960,	/* 98.304 MHz */
	610080582,	/* 99 MHz */
	610080582,	/* 99 MHz */
	615164587,	/* 98.18181818 MHz */
};

static const u32 cv18xx_pll_csr[] = {
	0x00128201,	/* * 9 / 2 = 442.368 MHz */
	0x00188101,	/* * 12 / 1 = 1188 MHz */
	0x00308201,	/* * 24 / 2 = 1188 MHz */
	0x00168101,	/* * 11 / 1 = 1080 MHz */
};

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

static void cv18xx_pll_init(void)
{
	int i;

	/* Run everything from the XTAL while the PLLs are reprogrammed */
	writel(0xffffffff, clkgen_reg(CLK_BYP_0));
	writel(0x0000003f, clkgen_reg(CLK_BYP_1));

	/* MIPIMPLL = 900 MHz */
	writel(0x05488101, clkgen_reg(MIPIMPLL_CSR));

	writel(0x3f, clkgen_reg(PLL_G2_SSC_SYN_CTRL));
	for (i = 0; i < ARRAY_SIZE(cv18xx_pll_csr); i++) {
		writel(cv18xx_pll_syn_set[i],
		       clkgen_reg(APLL_SSC_SYN_SET + 0x10 * i));
		clrsetbits_le32(clkgen_reg(APLL_SSC_SYN_CTRL + 0x10 * i),
				SSC_SYN_CTRL_BYPASS, SSC_SYN_CTRL_SW_UP);
		writel(cv18xx_pll_csr[i], clkgen_reg(APLL0_CSR + 4 * i));
	}

	/* Power up the PLLs */
	clrbits_le32(clkgen_reg(PLL_G2_CTRL), PLL_G2_CTRL_PD);

	/* MPLL = TPLL = 850 MHz */
	writel(0x00448101, clkgen_reg(MPLL_CSR));
	writel(0x00448101, clkgen_reg(TPLL_CSR));

	for (i = 0; i < ARRAY_SIZE(cv18xx_clk_divs); i++)
		writel(cv18xx_clk_divs[i].val,
		       clkgen_reg(cv18xx_clk_divs[i].off));

	clrbits_le32(clkgen_reg(MIPIMPLL_D3_PD), MIPIMPLL_D3_PD_PD);
	clrbits_le32(clkgen_reg(CAM0PLL_D2_PD), CAM0PLL_D2_PD_PD);

	/* Wait for the PLLs to lock */
	udelay(200);

	/* C906 clocks from the *_0 dividers, then leave the XTAL bypass */
	writel(CLK_SEL_0_C906_0 | CLK_SEL_0_C906_1, clkgen_reg(CLK_SEL_0));
	writel(0, clkgen_reg(CLK_BYP_0));
	writel(0, clkgen_reg(CLK_BYP_1));
}

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

	cv18xx_pll_init();
}
