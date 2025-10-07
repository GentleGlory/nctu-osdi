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



#define SPSR_EL2_DAIF_DISABLE	(7 << 6)

#define SPSR_EL2_M_MASK	(~(0b11111))
#define SPSR_EL2_M_EL1H	(0b0101)

#define CPACR_EL1_FPEN		(0b11 << 20)

#define SPSR_EL1_DAIF_MASK		(~(0b1111 << 6))
#define SPSR_EL1_DAIF_CLEAR_ALL	(0b0000 << 6)
#define SPSR_EL1_EL_MASK		(~(0b1111 << 0))	
#define SPSR_EL1_EL0			(0b0000 << 0)

#endif