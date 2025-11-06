#ifndef _AUX_PERI_H
#define _AUX_PERI_H

#include "core.h"

#define AUX_BASE					(MMIO_BASE + 0x00215000)
#define AUX_AUXENB_REG				(AUX_BASE + 0x4)
#define   AUX_AUXENB_MINI_UART		BIT(0)
#define   AUX_AUXENB_SPI_1			BIT(1)
#define   AUX_AUXENB_SPI_2			BIT(2)

#define AUX_MU_IO_REG				(AUX_BASE + 0x40)

#define AUX_MU_IER_REG				(AUX_BASE + 0x44)

#define AUX_MU_IIR_REG				(AUX_BASE + 0x48)
#define   AUX_MU_IIR_CLEAR_RX_FIFO	BIT(1)
#define   AUX_MU_IIR_CLEAR_TX_FIFO	BIT(2)

#define AUX_MU_LCR_REG				(AUX_BASE + 0x4C)
#define   AUX_MU_LCR_DATA_BIT_MASK	(GENMASK(1,0))
#define   AUX_MU_LCR_DATA_7_BIT		0x0
#define   AUX_MU_LCR_DATA_8_BIT		0x3

#define AUX_MU_MCR_REG				(AUX_BASE + 0x50)

#define AUX_MU_LSR_REG				(AUX_BASE + 0x54)
#define   AUX_MU_LSR_DATA_READY		BIT(0)
#define   AUX_MU_LSR_TX_EMPTY		BIT(5)

#define AUX_MU_CNTL_REG				(AUX_BASE + 0x60)
#define   AUX_MU_CNTL_RX_EN			BIT(0)
#define   AUX_MU_CNTL_TX_EN			BIT(1)

#define AUX_MU_BAUD					(AUX_BASE + 0x68)

#endif