// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026, Hiago De Franco <hfranco@baylibre.com>
 */

#include <asm/csr.h>
#include <linux/bitops.h>

#define CSR_MCOR			0x7c2
#define  CSR_MCOR_CACHE_INV		BIT(4)
#define  CSR_MCOR_CACHE_SEL_BOTH	(BIT(1) | BIT(0))
#define CSR_MHCR			0x7c1
#define  CSR_MHCR_DE			BIT(1)
#define  CSR_MHCR_IE			BIT(0)
#define CSR_MXSTATUS			0x7c0
#define  CSR_MXSTATUS_CLINTEE		BIT(17)
#define  CSR_MXSTATUS_MAEE		BIT(21)
#define  CSR_MXSTATUS_MM		BIT(15)
#define  CSR_MXSTATUS_THEADISAEE	BIT(22)
#define  CSR_MXSTATUS_UCME		BIT(16)

/* The C906 CSRs are only accessible in M-mode, so this is done in SPL only */
void harts_early_init(void)
{
	csr_write(CSR_MXSTATUS, CSR_MXSTATUS_THEADISAEE | CSR_MXSTATUS_MAEE |
		  CSR_MXSTATUS_CLINTEE | CSR_MXSTATUS_UCME | CSR_MXSTATUS_MM);
	csr_write(CSR_MCOR, CSR_MCOR_CACHE_INV | CSR_MCOR_CACHE_SEL_BOTH);
	csr_set(CSR_MHCR, CSR_MHCR_IE | CSR_MHCR_DE);
}
