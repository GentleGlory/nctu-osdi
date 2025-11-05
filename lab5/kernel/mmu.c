#include "mmu.h"
#include "page.h"
#include "string.h"

static void mmu_alloc_pte(pmd_t *pmdp, uint64_t addr,
		uint64_t end, uint64_t phys, uint64_t prot)
{
	pmd_t pmd = *pmdp;
	pte_t *ptep = NULL;
	
	if ( pmd == 0) {
		uint64_t pte_phys;
		pte_phys = (uint64_t)page_alloc_pgtable();

		if (pte_phys == 0) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pte\n");
			return;
		}

		*pmdp = (pte_phys | PD_TABLE);
		pmd = *pmdp;
	}

	ptep = (pte_t *)(pmd & PTE_ADDR_MASK) + pte_index(addr);

	do {
		*ptep = phys | prot;
		phys += PAGE_SIZE;
	} while (ptep ++, addr += PAGE_SIZE, addr != end);
}

static void mmu_alloc_pmd(pud_t *pudp, uint64_t addr,
		uint64_t end, uint64_t phys, uint64_t prot)
{
	pud_t pud = *pudp;
	pmd_t *pmdp = NULL;
	uint64_t next;

	if ( pud == 0) {
		uint64_t pmd_phys;
		pmd_phys = (uint64_t)page_alloc_pgtable();

		if (pmd_phys == 0) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pmd\n");
			return;
		}

		*pudp = (pmd_phys | PD_TABLE);
		pud = *pudp;
	}

	pmdp = (pmd_t *)(pud & PTE_ADDR_MASK) + pmd_index(addr);

	do {
		next = pmd_addr_end(addr, end);
		mmu_alloc_pte(pmdp, addr, next, phys,
			prot);
		
		phys += next - addr;
	} while (pmdp ++, addr = next, addr != end);
}

static void mmu_alloc_pud(pgd_t *pgdp, uint64_t addr,
		uint64_t end, uint64_t phys, uint64_t prot) 
{
	pgd_t pgd = *pgdp;
	pud_t *pudp = NULL;
	uint64_t next;

	if ( pgd == 0) {
		uint64_t pud_phys;
		pud_phys = (uint64_t)page_alloc_pgtable();

		if (pud_phys == 0) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pud\n");
			return;
		}

		*pgdp = (pud_phys | PD_TABLE);
		pgd = *pgdp;
	}

	pudp = (pud_t *)(pgd & PTE_ADDR_MASK) + pud_index(addr);

	do {
		next = pud_addr_end(addr, end);
		mmu_alloc_pmd(pudp, addr, next, phys,
			prot);
		
		phys += next - addr;
	} while (pudp ++, addr = next, addr != end);
}

void mmu_create_pgd_mapping(pgd_t *pgdir, uint64_t phys,
		uint64_t virt, uint64_t size, uint64_t prot)
{
	uint64_t addr, end, next;
	
	pgd_t *pgdp = pgd_offset_raw(pgdir, virt);

	phys &= PAGE_MASK;
	addr = virt & PAGE_MASK;
	end = PAGE_ALIGN(virt + size);

	do {
		next = pgd_addr_end(addr, end);
		mmu_alloc_pud(pgdp, addr, next, phys,
				prot);
		phys += next - addr;
	} while (pgdp++, addr = next, addr != end);
}
