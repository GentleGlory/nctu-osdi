#ifndef _SYS_REGS_H
#define _SYS_REGS_H

#define HCR_EL2_RW_AARCH64	(1 << 31)

#define SCTLR_ELx_M	(1<<0)
#define SCTLR_ELx_C	(1<<2) /*data cache enable*/

#define SCTLR_EL1_EE_MASK 	(~(1 << 25))
#define SCTLR_EL1_EE_LITTLE (0 << 25)

#define SCTLR_EL1_EOE_MASK		(~(1 << 24))
#define SCTLR_EL1_EOE_LITTLE	(0 << 24)
#define SCTLR_EL1_M_MASK		(~(1 << 0))
#define SCTLR_EL1_M_DISABLE_MMU	(0 << 0)



#define SPSR_EL2_DAIF_DISABLE		(7 << 6)

#define SPSR_EL2_M_MASK	(~(0b11111))
#define SPSR_EL2_M_EL1H	(0b0101)

#define CPACR_EL1_FPEN		(0b11 << 20)

#define SPSR_EL1_DAIF_MASK		(~(0b1111 << 6))
#define SPSR_EL1_DAIF_CLEAR_ALL	(0b0000 << 6)
#define SPSR_EL1_EL_MASK		(~(0b1111 << 0))	
#define SPSR_EL1_EL0			(0b0000 << 0)
#define SPSR_EL1_EL1			(0b0101 << 0)

#define TCR_CONFIG_REGION_48BIT	(((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB			((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT		(TCR_CONFIG_REGION_48BIT | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE			0b00000000
#define MAIR_NORMAL_NOCACHE			0b01000100
#define MAIR_IDX_DEVICE_nGnRnE		0
#define MAIR_IDX_NORMAL_NOCACHE		1

#endif