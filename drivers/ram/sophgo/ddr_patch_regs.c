// SPDX-License-Identifier: GPL-2.0+
/*
 * Sophgo CV181x DDR3-1866 x16 PHY patch settings
 *
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 *
 * Based on the Sophgo FSBL (https://github.com/sophgo/fsbl),
 * plat/cv181x/ddr/ddr_config/ddr3_1866_x16/ddr_patch_regs.c.
 */

#include "ddr_sys.h"

/* regpatch_ddr3_x16_bga.c for 1866 */
static const struct ddr_reg ddr_patch_sip[] = {
	/* tune damp */
	{ 0x150, 0x00000005 },

	/* CSB & CA driving */
	{ 0x97c, 0x08080404 },

	/* CLK driving */
	{ 0x980, 0x08080808 },

	/* BYTE0 DQ driving */
	{ 0xa38, 0x00000606 },
	/* BYTE0 DQS driving */
	{ 0xa3c, 0x06060606 },
	/* BYTE1 DQ driving */
	{ 0xa78, 0x00000606 },
	/* BYTE1 DQS driving */
	{ 0xa7c, 0x06060606 },

	/*
	 * trigger level
	 * BYTE0
	 */
	{ 0xb24, 0x00100010 },
	/* BYTE1 */
	{ 0xb54, 0x00100010 },

	/*
	 * APHY TX VREFDQ rangex2 [1]
	 * VREF DQ
	 */
	{ 0x410, 0x00120002 },
	/*
	 * APHY TX VREFCA rangex2 [1]
	 * VREF CA
	 */
	{ 0x414, 0x00100002 },

	/*
	 * tx dline code
	 * BYTE0 DQ
	 */
	{ 0xa00, 0x06430643 },
	{ 0xa04, 0x06430643 },
	{ 0xa08, 0x06430643 },
	{ 0xa0c, 0x06430643 },
	{ 0xa10, 0x00000643 },
	{ 0xa14, 0x0a7e007e },
	/* BYTE1 DQ */
	{ 0xa40, 0x06480648 },
	{ 0xa44, 0x06480648 },
	{ 0xa48, 0x06480648 },
	{ 0xa4c, 0x06480648 },
	{ 0xa50, 0x00000648 },
	{ 0xa54, 0x0a7e007e },

	/*
	 * APHY RX TRIG rangex2[18] & disable lsmode[0]
	 * f0_param_phya_reg_rx_byte0_en_lsmode[0]
	 * f0_param_phya_reg_byte0_en_rec_vol_mode[12]
	 * f0_param_phya_reg_rx_byte0_force_en_lvstl_odt[16]
	 * f0_param_phya_reg_rx_byte0_sel_dqs_rec_vref_mode[8]
	 * param_phya_reg_rx_byte0_en_trig_lvl_rangex2[18]
	 * BYTE0 [0]
	 */
	{ 0x500, 0x00041001 },
	/*
	 * f0_param_phya_reg_rx_byte1_en_lsmode[0]
	 * f0_param_phya_reg_byte1_en_rec_vol_mode[12]
	 * f0_param_phya_reg_rx_byte0_force_en_lvstl_odt[16]
	 * f0_param_phya_reg_rx_byte0_sel_dqs_rec_vref_mode[8]
	 * param_phya_reg_rx_byte0_en_trig_lvl_rangex2[18]
	 * BYTE1 [0]
	 */
	{ 0x540, 0x00041001 },

	/*
	 * FOR U02
	 * U02 enable DQS voltage mode receiver
	 * f0_param_phya_reg_tx_byte0_en_tx_de_dqs[20]
	 */
	{ 0x504, 0x00100000 },
	/* f0_param_phya_reg_tx_byte1_en_tx_de_dqs[20] */
	{ 0x544, 0x00100000 },
	/*
	 * U02 enable MASK voltage mode receiver
	 * param_phya_reg_rx_sel_dqs_wo_pream_mode[2]
	 */
	{ 0x138, 0x00000014 },

	/* BYTE0 RX DQ deskew */
	{ 0xb00, 0x00020402 },
	{ 0xb04, 0x05020401 },
	/* BYTE0  DQ8 deskew [6:0] neg DQS  [15:8]  ;  pos DQS  [23:16] */
	{ 0xb08, 0x00313902 },

	/* BYTE1 RX DQ deskew */
	{ 0xb30, 0x06000100 },
	{ 0xb34, 0x02010303 },
	/* BYTE1  DQ8 deskew [6:0] neg DQS  [15:8]  ;  pos DQS  [23:16] */
	{ 0xb38, 0x00323900 },

	/*
	 * Read gate TX dline + shift
	 * BYTE0
	 */
	{ 0xb0c, 0x00000a14 },
	/* BYTE1 */
	{ 0xb3c, 0x00000a14 },

	/* CKE dline + shift CKE0 [6:0]+[13:8] ; CKE1 [22:16]+[29:24] */
	{ 0x930, 0x04000400 },
	/* CSB dline + shift CSB0 [6:0]+[13:8] ; CSB1 [22:16]+[29:24] */
	{ 0x934, 0x04000400 },
};

/* External DDR3: same as SIP but disable DQS voltage mode RX */
static const struct ddr_reg ddr_patch_ext[] = {
	/* tune damp */
	{ 0x150, 0x00000005 },

	/* CSB & CA driving */
	{ 0x97c, 0x08080404 },

	/* CLK driving */
	{ 0x980, 0x08080808 },

	/* BYTE0 DQ driving */
	{ 0xa38, 0x00000606 },
	/* BYTE0 DQS driving */
	{ 0xa3c, 0x06060606 },
	/* BYTE1 DQ driving */
	{ 0xa78, 0x00000606 },
	/* BYTE1 DQS driving */
	{ 0xa7c, 0x06060606 },

	/*
	 * trigger level
	 * BYTE0
	 */
	{ 0xb24, 0x00100010 },
	/* BYTE1 */
	{ 0xb54, 0x00100010 },

	/*
	 * APHY TX VREFDQ rangex2 [1]
	 * VREF DQ
	 */
	{ 0x410, 0x00120002 },
	/*
	 * APHY TX VREFCA rangex2 [1]
	 * VREF CA
	 */
	{ 0x414, 0x00100002 },

	/*
	 * tx dline code
	 * BYTE0 DQ
	 */
	{ 0xa00, 0x06430643 },
	{ 0xa04, 0x06430643 },
	{ 0xa08, 0x06430643 },
	{ 0xa0c, 0x06430643 },
	{ 0xa10, 0x00000643 },
	{ 0xa14, 0x0a7e007e },
	/* BYTE1 DQ */
	{ 0xa40, 0x06480648 },
	{ 0xa44, 0x06480648 },
	{ 0xa48, 0x06480648 },
	{ 0xa4c, 0x06480648 },
	{ 0xa50, 0x00000648 },
	{ 0xa54, 0x0a7e007e },

	/*
	 * APHY RX TRIG rangex2[18] & disable lsmode[0]
	 * f0_param_phya_reg_rx_byte0_en_lsmode[0]
	 * f0_param_phya_reg_byte0_en_rec_vol_mode[12]
	 * f0_param_phya_reg_rx_byte0_force_en_lvstl_odt[16]
	 * f0_param_phya_reg_rx_byte0_sel_dqs_rec_vref_mode[8]
	 * param_phya_reg_rx_byte0_en_trig_lvl_rangex2[18]
	 * BYTE0 [0]
	 */
	{ 0x500, 0x00040001 },
	/*
	 * f0_param_phya_reg_rx_byte1_en_lsmode[0]
	 * f0_param_phya_reg_byte1_en_rec_vol_mode[12]
	 * f0_param_phya_reg_rx_byte0_force_en_lvstl_odt[16]
	 * f0_param_phya_reg_rx_byte0_sel_dqs_rec_vref_mode[8]
	 * param_phya_reg_rx_byte0_en_trig_lvl_rangex2[18]
	 * BYTE1 [0]
	 */
	{ 0x540, 0x00040001 },

	/*
	 * FOR U02
	 * U02 DQS voltage mode receiver -- disabled for external DDR
	 * f0_param_phya_reg_tx_byte0_en_tx_de_dqs[20]
	 */
	{ 0x504, 0x00000000 },
	/* f0_param_phya_reg_tx_byte1_en_tx_de_dqs[20] */
	{ 0x544, 0x00000000 },
	/*
	 * U02 enable MASK voltage mode receiver
	 * param_phya_reg_rx_sel_dqs_wo_pream_mode[2]
	 */
	{ 0x138, 0x00000014 },

	/* RX ODT 120ohm for external DDR3 (override ddr_init.h default 240ohm) */
	{ 0x41c, 0x00004242 },

	/* BYTE0 RX DQ deskew */
	{ 0xb00, 0x00020402 },
	{ 0xb04, 0x05020401 },
	/* BYTE0  DQ8 deskew [6:0] neg DQS  [15:8]  ;  pos DQS  [23:16] */
	{ 0xb08, 0x00313902 },

	/* BYTE1 RX DQ deskew */
	{ 0xb30, 0x06000100 },
	{ 0xb34, 0x02010303 },
	/* BYTE1  DQ8 deskew [6:0] neg DQS  [15:8]  ;  pos DQS  [23:16] */
	{ 0xb38, 0x00323900 },

	/*
	 * Read gate TX dline + shift
	 * BYTE0
	 */
	{ 0xb0c, 0x00000a14 },
	/* BYTE1 */
	{ 0xb3c, 0x00000a14 },

	/* CKE dline + shift CKE0 [6:0]+[13:8] ; CKE1 [22:16]+[29:24] */
	{ 0x930, 0x04000400 },
	/* CSB dline + shift CSB0 [6:0]+[13:8] ; CSB1 [22:16]+[29:24] */
	{ 0x934, 0x04000400 },
};

void ddr_patch_set(u8 vendor)
{
	const struct ddr_reg *r = ddr_patch_sip;
	const struct ddr_reg *end = r + ARRAY_SIZE(ddr_patch_sip);

	if (vendor == DDR_EXTERN_DDR3) {
		r = ddr_patch_ext;
		end = r + ARRAY_SIZE(ddr_patch_ext);
	}

	for (; r < end; r++)
		writel(r->val, PHYD_BASE + r->off);
}
