#include "sys_regs.h"
#include "mmu.h"


.section ".text"
.align	3
.global enable_mmu
enable_mmu:
	//Setting tcr and mair
	ldr x0, =TCR_CONFIG_DEFAULT
	msr tcr_el1, x0
	
	ldr x0, =( \
			(MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
			(MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
			)
	msr mair_el1, x0

	//Setup pgd
	adrp x0, __id_map_pgd_start
	add x0, x0, #:lo12:__id_map_pgd_start

	adrp x1, __id_map_pud_start
	add x1, x1, #:lo12:__id_map_pud_start

	// combine the physical address of next level page with attribute.
	ldr x2, =BOOT_PGD_ATTR
	orr x2, x1, x2
	str x2, [x0]

	ldr x2, =BOOT_PUD_ATTR
	adrp x3, __id_map_pmd_start
	add x3, x3, #:lo12:__id_map_pmd_start
	orr x3, x2, x3
	str x3, [x1]

	adrp x3, __id_map_pmd_start
	add x3, x3, #:lo12:__id_map_pmd_start
	add x3, x3, 0x1000
	orr x3, x2, x3
	str x3, [x1, #8]

	ldr x0, =BOOT_VA_START
	ldr x1, =BOOT_VA_START
	ldr x2, =(BOOT_VA_START + BOOT_GPU_PERI_BASE - 1)
	ldr x3, =MAIR_NORMAL_NOCACHE
	bl mapping_pmd


	ldr x0, =(BOOT_VA_START + BOOT_GPU_PERI_BASE)
	ldr x1, =(BOOT_VA_START + BOOT_GPU_PERI_BASE)
	ldr x2, =(BOOT_VA_START + BOOT_GPU_PERI_BASE + BOOT_GPU_PERI_SIZE - 1)
	ldr x3, =MAIR_DEVICE_nGnRnE
	bl mapping_pmd

	ldr x0, =(BOOT_VA_START + BOOT_LOCAL_PERI_BASE)
	ldr x1, =(BOOT_VA_START + BOOT_LOCAL_PERI_BASE)
	ldr x2, =(BOOT_VA_START + BOOT_LOCAL_PERI_BASE + BOOT_LOCAL_PERI_SIZE - 1)
	ldr x3, =MAIR_DEVICE_nGnRnE
	bl mapping_pmd

	dsb ishst
	
	adrp x0, __id_map_pgd_start
	add x0, x0, #:lo12:__id_map_pgd_start

	msr ttbr0_el1, x0
	msr ttbr1_el1, x0
	dsb ish
	isb

	mrs x2, sctlr_el1
	orr x2, x2 , 1
	msr sctlr_el1, x2
	isb

	adr x0, mmu_started
	ldr x1, =__kernel_virtual_base
	add x0, x1, x0
	br x0


.align	3
//x0: phy_start, x1:virtual_start, x2: virtual_end, x3 mair
mapping_pmd:
	//Set x4 as (BOOT_PMD_ATTR | mair << 2)
	ldr x4, =BOOT_PMD_ATTR
	lsl x3, x3, #2
	orr x4, x4, x3

	//Setup PMD dir base. 
	adrp x5, __id_map_pmd_start
	add x5, x5, #:lo12:__id_map_pmd_start

	mov x8, x0
	ubfx x8, x8, #PUD_SHIFT, #PUD_BIT_SIZE
	add x5, x5, x8, lsl #12

1:	cmp x1, x2
	b.ls 2f
	b 3f
2:	//Get pmd num
	mov x6, x0
	ubfx x6, x6, #PMD_SHIFT, #PMD_BIT_SIZE

	mov x7, x0
	orr x7, x4, x7
	str x7, [x5, x6, lsl #3]

	add x0, x0, #PMD_SIZE
	add x1, x1, #PMD_SIZE

	b 1b

3:	ret