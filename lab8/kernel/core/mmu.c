#include "mmu.h"
#include "page.h"
#include "string.h"
#include "kmalloc.h"

static inline int mmu_entry_is_table(uint64_t entry)
{
	return (entry & PD_TABLE) == PD_TABLE;
}

static inline int mmu_entry_is_block(uint64_t entry)
{
	return ((entry & PD_BLOCK) == PD_BLOCK) && !mmu_entry_is_table(entry);
}

static inline uint64_t mmu_entry_phys(uint64_t entry)
{
	return entry & PTE_ADDR_MASK;
}

static inline void *mmu_entry_to_kva(uint64_t entry)
{
	return PHYS_TO_KERNEL_VIRT(mmu_entry_phys(entry));
}

static inline uint64_t mmu_entry_attrs(uint64_t entry)
{
	return entry & ~PTE_ADDR_MASK;
}

static inline int mmu_leaf_from_entry(uint64_t entry, uint64_t virt_addr,
		uint64_t level_shift, uint64_t *phys, size_t *chunk)
{
	uint64_t block_size = 1ULL << level_shift;
	uint64_t offset = virt_addr & (block_size - 1);

	*phys = mmu_entry_phys(entry) + offset;
	*chunk = block_size - offset;
	return 0;
}

static void mmu_report_leaf(const char *level, uint64_t entry,
			uint64_t virt_addr, uint64_t shift)
{
	uint64_t offset_mask = (1ULL << shift) - 1;
	uint64_t phys = mmu_entry_phys(entry) | (virt_addr & offset_mask);

	printf("\r[%s] leaf: VA 0x%llx -> PA 0x%llx (attr 0x%llx)\n",
		level, virt_addr, phys, mmu_entry_attrs(entry));
}

static void mmu_report_invalid(const char *level, uint64_t idx,
			uint64_t virt_addr)
{
	printf("\r[%s] entry %llu invalid for VA 0x%llx\n",
		level, idx, virt_addr);
}

static int mmu_translate_va(pgd_t *pgdir, uint64_t virt_addr,
		uint64_t *phys_addr, size_t *chunk)
{
	pgd_t pgd_entry = pgdir[pgd_index(virt_addr)];

	if (pgd_entry == 0)
		return -1;

	if (!mmu_entry_is_table(pgd_entry))
		return mmu_leaf_from_entry(pgd_entry, virt_addr, PGD_SHIFT,
			phys_addr, chunk);

	pud_t *pud_table = (pud_t *)mmu_entry_to_kva(pgd_entry);
	pud_t pud_entry = pud_table[pud_index(virt_addr)];

	if (pud_entry == 0)
		return -1;

	if (mmu_entry_is_block(pud_entry))
		return mmu_leaf_from_entry(pud_entry, virt_addr, PUD_SHIFT,
			phys_addr, chunk);

	pmd_t *pmd_table = (pmd_t *)mmu_entry_to_kva(pud_entry);
	pmd_t pmd_entry = pmd_table[pmd_index(virt_addr)];

	if (pmd_entry == 0)
		return -1;

	if (mmu_entry_is_block(pmd_entry))
		return mmu_leaf_from_entry(pmd_entry, virt_addr, PMD_SHIFT,
			phys_addr, chunk);

	pte_t *pte_table = (pte_t *)mmu_entry_to_kva(pmd_entry);
	pte_t pte_entry = pte_table[pte_index(virt_addr)];

	if (pte_entry == 0)
		return -1;

	return mmu_leaf_from_entry(pte_entry, virt_addr, PAGE_SHIFT,
			phys_addr, chunk);
}

static void mmu_alloc_pte(pmd_t *pmdp, uint64_t addr,
		uint64_t end, uint64_t phys, uint64_t prot)
{
	pmd_t pmd = *pmdp;
	pte_t *ptep = NULL;
	
	if ( pmd == 0) {
		void *pte_page = kmalloc_alloc(PAGE_SIZE);
		uint64_t pte_phys;

		if (pte_page == NULL) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pte\n");
			return;
		}

		memset(pte_page, 0, PAGE_SIZE);
		pte_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pte_page);
		*pmdp = (pte_phys | PD_TABLE);
		pmd = *pmdp;
	}

	ptep = (pte_t *)PHYS_TO_KERNEL_VIRT(pmd & PTE_ADDR_MASK);
	ptep += pte_index(addr);

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
		void *pmd_page = kmalloc_alloc(PAGE_SIZE);
		uint64_t pmd_phys;

		if (pmd_page == NULL) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pmd\n");
			return;
		}

		memset(pmd_page, 0, PAGE_SIZE);
		pmd_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pmd_page);
		*pudp = (pmd_phys | PD_TABLE);
		pud = *pudp;
	}

	pmdp = (pmd_t *)PHYS_TO_KERNEL_VIRT(pud & PTE_ADDR_MASK);
	pmdp += pmd_index(addr);

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
		void *pud_page = kmalloc_alloc(PAGE_SIZE);
		uint64_t pud_phys;

		if (pud_page == NULL) {
			printf("\rFailed to alloc pgtable in mmu_alloc_pud\n");
			return;
		}

		memset(pud_page, 0, PAGE_SIZE);
		pud_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pud_page);
		*pgdp = (pud_phys | PD_TABLE);
		pgd = *pgdp;
	}

	pudp = (pud_t *)PHYS_TO_KERNEL_VIRT(pgd & PTE_ADDR_MASK);
	pudp += pud_index(addr);

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

void mmu_walk_page_table(pgd_t *pgdir, uint64_t virt_addr)
{
	uint64_t pgd_idx = pgd_index(virt_addr);
	uint64_t pud_idx = pud_index(virt_addr);
	uint64_t pmd_idx = pmd_index(virt_addr);
	uint64_t pte_idx = pte_index(virt_addr);
	pgd_t pgd_entry = pgdir[pgd_idx];

	printf("\r[MMU] walk VA 0x%llx\n", virt_addr);
	printf("\r  PGD[%llu] = 0x%llx\n", pgd_idx, pgd_entry);

	if (pgd_entry == 0) {
		mmu_report_invalid("PGD", pgd_idx, virt_addr);
		return;
	}

	if (!mmu_entry_is_table(pgd_entry)) {
		mmu_report_leaf("PGD", pgd_entry, virt_addr, PGD_SHIFT);
		return;
	}

	pud_t *pud_table = (pud_t *)mmu_entry_to_kva(pgd_entry);
	pud_t pud_entry = pud_table[pud_idx];
	printf("\r  PUD[%llu] = 0x%llx\n", pud_idx, pud_entry);

	if (pud_entry == 0) {
		mmu_report_invalid("PUD", pud_idx, virt_addr);
		return;
	}

	if (mmu_entry_is_block(pud_entry)) {
		mmu_report_leaf("PUD", pud_entry, virt_addr, PUD_SHIFT);
		return;
	}

	pmd_t *pmd_table = (pmd_t *)mmu_entry_to_kva(pud_entry);
	pmd_t pmd_entry = pmd_table[pmd_idx];
	printf("\r  PMD[%llu] = 0x%llx\n", pmd_idx, pmd_entry);

	if (pmd_entry == 0) {
		mmu_report_invalid("PMD", pmd_idx, virt_addr);
		return;
	}

	if (mmu_entry_is_block(pmd_entry)) {
		mmu_report_leaf("PMD", pmd_entry, virt_addr, PMD_SHIFT);
		return;
	}

	pte_t *pte_table = (pte_t *)mmu_entry_to_kva(pmd_entry);
	pte_t pte_entry = pte_table[pte_idx];
	printf("\r  PTE[%llu] = 0x%llx\n", pte_idx, pte_entry);

	if (pte_entry == 0) {
		mmu_report_invalid("PTE", pte_idx, virt_addr);
		return;
	}

	mmu_report_leaf("PTE", pte_entry, virt_addr, PAGE_SHIFT);
}

int mmu_copy_to_user(pgd_t *pgdir, uint64_t user_dst, const void *kernel_src,
		size_t len)
{
	size_t copied = 0;
	const unsigned char *src = (const unsigned char *)kernel_src;

	while (copied < len) {
		uint64_t phys = 0;
		size_t chunk = 0;
		uint64_t cur = user_dst + copied;

		if (mmu_translate_va(pgdir, cur, &phys, &chunk) < 0)
			return -1;

		if (chunk > (len - copied))
			chunk = len - copied;

		void *mapped = PHYS_TO_KERNEL_VIRT(phys);
		memcpy(mapped, src + copied, chunk);

		copied += chunk;
	}

	return 0;
}

int mmu_copy_from_user(pgd_t *pgdir, void *kernel_dst, uint64_t user_src,
		size_t len)
{
	size_t copied = 0;
	unsigned char *dst = (unsigned char *)kernel_dst;

	while (copied < len) {
		uint64_t phys = 0;
		size_t chunk = 0;
		uint64_t cur = user_src + copied;

		if (mmu_translate_va(pgdir, cur, &phys, &chunk) < 0)
			return -1;

		if (chunk > (len - copied))
			chunk = len - copied;

		void *mapped = PHYS_TO_KERNEL_VIRT(phys);
		memcpy(dst + copied, mapped, chunk);

		copied += chunk;
	}

	return 0;
}

static void mmu_pte_free(pte_t *pte) 
{
	pte_t *pte_tmp = pte;
	uint64_t phys;
	for (int i = 0; i < PTRS_PER_PTE; i++) {
		if ((*pte) != 0) {
			phys = (*pte) & PTE_ADDR_MASK;
			kmalloc_free(PHYS_TO_KERNEL_VIRT(phys));
		}

		pte ++;
	}

	kmalloc_free(pte_tmp);
}

static void mmu_pmd_free(pmd_t *pmd) 
{
	pmd_t *pmd_tmp = pmd;
	uint64_t phys;
	
	pte_t *pte = NULL;
	for (int i = 0; i < PTRS_PER_PMD; i++) {
		if ((*pmd) != 0) {
			phys = (*pmd) & PTE_ADDR_MASK;
			pte = (pte_t *) PHYS_TO_KERNEL_VIRT(phys);
			mmu_pte_free(pte);
		}

		pmd ++;
	}

	kmalloc_free(pmd_tmp);
}

static void mmu_pud_free(pud_t *pud) 
{	
	pud_t *pud_tmp = pud;
	uint64_t phys;
	pmd_t *pmd = NULL;
	
	for (int i = 0; i < PTRS_PER_PUD; i++) {
		if ((*pud) != 0) {
			phys = (*pud) & PTE_ADDR_MASK;
			pmd = (pmd_t *) PHYS_TO_KERNEL_VIRT(phys);
			mmu_pmd_free(pmd);
		}

		pud ++;
	}

	kmalloc_free(pud_tmp);
}

void mmu_pgd_free(pgd_t *pgd)
{
	pgd_t *pgd_tmp = pgd;
	uint64_t phys;
	pud_t *pud = NULL;
	for (int i = 0; i < PTRS_PER_PGD; i++) {
		if ((*pgd) != 0) {
			phys = (*pgd) & PTE_ADDR_MASK;
			pud = (pud_t *) PHYS_TO_KERNEL_VIRT(phys);
			mmu_pud_free(pud);
		}

		pgd ++;
	}

	kmalloc_free(pgd_tmp);
}

static int mmu_copy_pte(pte_t *pte_dest, pte_t *pte_src)
{
	for (int i = 0; i < PTRS_PER_PTE; i++) {
		pte_t entry = pte_src[i];

		if (entry == 0) {
			pte_dest[i] = 0;
			continue;
		}

		void *dst = kmalloc_alloc(PAGE_SIZE);
		if (dst == NULL)
			goto fail;

		void *src = mmu_entry_to_kva(entry);

		memcpy(dst, src, PAGE_SIZE);

		pte_dest[i] = KERNEL_VIRT_TO_PHYS(dst) | mmu_entry_attrs(entry);
	}

	return 0;

fail:
	for (int i = 0; i < PTRS_PER_PTE; i++) {
		if (pte_dest[i] != 0) {
			uint64_t phys = mmu_entry_phys(pte_dest[i]);
			kmalloc_free(PHYS_TO_KERNEL_VIRT(phys));
			pte_dest[i] = 0;
		}
	}
	return -1;
}

static int mmu_copy_pmd(pmd_t *pmd_dest, pmd_t *pmd_src)
{
	for (int i = 0; i < PTRS_PER_PMD; i++) {
		pmd_t entry = pmd_src[i];

		if (entry == 0) {
			pmd_dest[i] = 0;
			continue;
		}

		if (!mmu_entry_is_table(entry)) {
			printf("\rmmu_copy_pmd: unsupported PMD entry type\n");
			goto fail;
		}

		void *pte_page = kmalloc_alloc(PAGE_SIZE);
		if (pte_page == NULL)
			goto fail;

		memset(pte_page, 0, PAGE_SIZE);

		pte_t *pte_dest = (pte_t *)pte_page;
		pte_t *pte_src = (pte_t *)mmu_entry_to_kva(entry);

		if (mmu_copy_pte(pte_dest, pte_src)) {
			mmu_pte_free(pte_dest);
			goto fail;
		}

		uint64_t pte_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pte_page);
		pmd_dest[i] = pte_phys | mmu_entry_attrs(entry);
	}

	return 0;

fail:
	for (int i = 0; i < PTRS_PER_PMD; i++) {
		if (pmd_dest[i] != 0) {
			pte_t *pte = (pte_t *)mmu_entry_to_kva(pmd_dest[i]);
			mmu_pte_free(pte);
			pmd_dest[i] = 0;
		}
	}
	return -1;
}

static int mmu_copy_pud(pud_t *pud_dest, pud_t *pud_src)
{
	for (int i = 0; i < PTRS_PER_PUD; i++) {
		pud_t entry = pud_src[i];

		if (entry == 0) {
			pud_dest[i] = 0;
			continue;
		}

		if (!mmu_entry_is_table(entry)) {
			printf("\rmmu_copy_pud: unsupported PUD entry type\n");
			goto fail;
		}

		void *pmd_page = kmalloc_alloc(PAGE_SIZE);
		if (pmd_page == NULL)
			goto fail;
		
		memset(pmd_page, 0, PAGE_SIZE);

		pmd_t *pmd_dest = (pmd_t *)pmd_page;
		pmd_t *pmd_src = (pmd_t *)mmu_entry_to_kva(entry);

		if (mmu_copy_pmd(pmd_dest, pmd_src)) {
			mmu_pmd_free(pmd_dest);
			goto fail;
		}

		uint64_t pmd_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pmd_page);
		pud_dest[i] = pmd_phys | mmu_entry_attrs(entry);
	}

	return 0;

fail:
	for (int i = 0; i < PTRS_PER_PUD; i++) {
		if (pud_dest[i] != 0) {
			pmd_t *pmd = (pmd_t *)mmu_entry_to_kva(pud_dest[i]);
			mmu_pmd_free(pmd);
			pud_dest[i] = 0;
		}
	}
	return -1;
}

int mmu_copy_pgd(pgd_t *pgdir_dest, pgd_t *pgdir_src)
{
	for (int i = 0; i < PTRS_PER_PGD; i++) {
		pgd_t entry = pgdir_src[i];

		if (entry == 0) {
			pgdir_dest[i] = 0;
			continue;
		}

		if (!mmu_entry_is_table(entry)) {
			printf("\rmmu_copy_pgd: unsupported PGD entry type\n");
			goto fail;
		}

		void *pud_page = kmalloc_alloc(PAGE_SIZE);
		if (pud_page == NULL)
			goto fail;

		memset(pud_page, 0, PAGE_SIZE);

		pud_t *pud_dest = (pud_t *)pud_page;
		pud_t *pud_src = (pud_t *)mmu_entry_to_kva(entry);

		if (mmu_copy_pud(pud_dest, pud_src)) {
			mmu_pud_free(pud_dest);
			goto fail;
		}

		uint64_t pud_phys = KERNEL_VIRT_TO_PHYS((uint64_t)pud_page);
		pgdir_dest[i] = pud_phys | mmu_entry_attrs(entry);
	}

	return 0;

fail:
	for (int i = 0; i < PTRS_PER_PGD; i++) {
		if (pgdir_dest[i] != 0) {
			pud_t *pud = (pud_t *)mmu_entry_to_kva(pgdir_dest[i]);
			mmu_pud_free(pud);
			pgdir_dest[i] = 0;
		}
	}
	return -1;
}
