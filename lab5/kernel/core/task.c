#include "task.h"
#include "string.h"
#include "sys_regs.h"
#include "scheduler.h"
#include "exception.h"
#include "context.h"
#include "lock.h"
#include "delay.h"
#include "mmu.h"


struct task task_pool[TASK_POOL_SIZE] __attribute__((aligned(16)));
struct task *idle_task = &task_pool[0];

extern unsigned char __kernel_stack_start;

#define USER_ADDITIONAL_MAPPING_SIZE	(PAGE_SIZE * 4)
#define USER_STACK_PAGES				4

void task_init()
{
	unsigned char *kernel_sp = NULL;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;
	
	// Initialize idle task

	idle_task->task_id = 0;
	idle_task->state = RUNNING;
	idle_task->need_reschedule = 1;
	idle_task->user_pgd_page = NULL;
	memset(&idle_task->cpu_context, 0, sizeof(struct cpu_context));

	// Initialize task pool
	for (int i = 1; i < TASK_POOL_SIZE; i++) {
		task_pool[i].task_id = i;
		task_pool[i].state = EXIT;
		INIT_LIST_HEAD(&task_pool[i].list);

		kernel_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * (i + 1) - 1;
		task_pool[i].reserved_kernel_sp = ((uint64_t) kernel_sp) & (~((uint64_t)0xf));

		task_pool[i].reserved_user_sp = USER_STACK_VIRT_ADDR;
		task_pool[i].user_pgd_page = NULL;
	}
}

void task_privilege_task_create(void(*func)(),
		enum task_priority priority)
{
	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			struct task *t = &task_pool[i];

			// Clear all CPU context registers
			memset(&t->cpu_context, 0, sizeof(struct cpu_context));

			// Set program counter to task function
			t->cpu_context.pc = (uint64_t) func;

			// Set up stack pointer (aligned to 16 bytes)
			t->cpu_context.sp = t->reserved_kernel_sp;
			
			// Mark task as ready to run
			// IMPORTANT: Must set state explicitly to RUNNING (0) to prevent compiler
			// from assuming it's already 0 and optimizing away this assignment
			t->need_reschedule = 0;
			t->epoch = DEFAULT_EPOCH;
			t->priority = priority;
			t->runnable_task_parent = NULL;

			// Force state assignment to happen by using volatile cast
			*((volatile enum task_state *)&t->state) = RUNNING;

			scheduler_add_task_to_queue(t, RUNNABLE_TASK_CURRENT);
			break;
		}
	}
}

void task_do_exec(uint64_t binary_start, uint64_t binary_size,
	uint64_t virtual_addr_start)
{
	struct page *pgd = NULL;
	pgd_t *pgdir = NULL;
	struct page *mem = NULL;
	struct task *cur = current;
	uint64_t binary_end = binary_start + binary_size;
	uint64_t binary_virtual_addr = virtual_addr_start;
	uint64_t mem_virtual_addr = 0, mem_phys_addr = 0;
	uint64_t size = 0;
	
	pgd = page_alloc();
	if (pgd == NULL) {
		printf("\rFailed to alloc pgd\n");
		return;
	}

	pgdir = (pgd_t *)PHYS_TO_KERNEL_VIRT(PAGE_FRAME_NUM_TO_PHYS(pgd->page_num));

	for (uint64_t i = binary_start; i < binary_end; i += PAGE_SIZE, binary_virtual_addr += PAGE_SIZE) {
		
		mem = page_alloc();

		if (mem == NULL) {
			printf("\rFailed to alloc page to copy binary\n");
			goto alloc_page_err;
		}

		size = (binary_end - i) > PAGE_SIZE ? PAGE_SIZE : binary_end - i;

		mem_phys_addr = PAGE_FRAME_NUM_TO_PHYS(mem->page_num);
		mem_virtual_addr = (uint64_t)PHYS_TO_KERNEL_VIRT(mem_phys_addr);

		memcpy((void *) mem_virtual_addr, (void *) i, size);

		mmu_create_pgd_mapping(pgdir, mem_phys_addr, binary_virtual_addr,
				size, (USER_PTE_ATTR | MAIR_IDX_NORMAL_NOCACHE << 2));
	}

	/* Reserve extra zeroed space for .bss/heap usage */
	uint64_t extra_virtual_addr = PAGE_ALIGN(binary_virtual_addr);
	uint64_t extra_virtual_end = extra_virtual_addr + USER_ADDITIONAL_MAPPING_SIZE;

	for (; extra_virtual_addr < extra_virtual_end; extra_virtual_addr += PAGE_SIZE) {
		mem = page_alloc();

		if (mem == NULL) {
			printf("\rFailed to alloc page for user extra mapping\n");
			goto alloc_page_err;
		}

		mem_phys_addr = PAGE_FRAME_NUM_TO_PHYS(mem->page_num);
		mem_virtual_addr = (uint64_t)PHYS_TO_KERNEL_VIRT(mem_phys_addr);
		//memset((void *)mem_virtual_addr, 0, PAGE_SIZE);

		mmu_create_pgd_mapping(pgdir, mem_phys_addr, extra_virtual_addr,
				PAGE_SIZE, (USER_PTE_ATTR | MAIR_IDX_NORMAL_NOCACHE << 2));
	}

	/* Create user stack (multiple pages) */
	uint64_t stack_base = cur->reserved_user_sp - (USER_STACK_PAGES * PAGE_SIZE);
	for (uint64_t stack_addr = stack_base; stack_addr < cur->reserved_user_sp; stack_addr += PAGE_SIZE) {
		mem = page_alloc();
		if (mem == NULL) {
			printf("\rFailed to alloc page for user stack\n");
			goto alloc_page_err;
		}

		mem_phys_addr = PAGE_FRAME_NUM_TO_PHYS(mem->page_num);
		mem_virtual_addr = (uint64_t)PHYS_TO_KERNEL_VIRT(mem_phys_addr);
		memset((void *)mem_virtual_addr, 0, PAGE_SIZE);

		mmu_create_pgd_mapping(pgdir, mem_phys_addr, stack_addr,
				PAGE_SIZE, (USER_PTE_ATTR | MAIR_IDX_NORMAL_NOCACHE << 2));
	}

	cur->user_pgd_page = pgd;

	ret_to_user(virtual_addr_start);
	//Should not be here
	return;
alloc_page_err:
	mmu_pgd_free(pgd);
}

void task_exit(int status)
{
	uint64_t irq_state;
	struct task *cur = NULL;
	
	irq_state = lock_irq_save();
	
	cur = current;

	cur->state = ZOMBIE;
	cur->need_reschedule = 1;
	scheduler_remove_task_from_queue(cur);

	if (cur->user_pgd_page != NULL) {
		mmu_pgd_free(cur->user_pgd_page);
		cur->user_pgd_page = NULL;
	}

	lock_irq_restore(irq_state);

	scheduler_do_schedule();
}

void task_zombie_reaper()
{	
	while (1) {
		
		for (int i = 0; i < TASK_POOL_SIZE; i++) {
			
			if (task_pool[i].state == ZOMBIE) {
				struct task *t = &task_pool[i];
				memset(&t->cpu_context, 0, sizeof(struct cpu_context));
				
				t->need_reschedule = 0;
				t->epoch = DEFAULT_EPOCH;
				t->priority = PRIORITY_LOW;
				t->runnable_task_parent = NULL;
				INIT_LIST_HEAD(&t->list);
				// Force state assignment to happen by using volatile cast
				*((volatile enum task_state *)&t->state) = EXIT;
			}
		}
		delay(5000);
	}
}

//Used by fork.
void task_prepare_fork(struct pt_regs *src_pt_regs, uint64_t sp_address, 
	uint64_t lr_address)
{
	uint64_t irq_state;
	struct task *dest = NULL;
	struct task *src = current;
	unsigned char *dest_sp = NULL;
	unsigned char *src_sp = NULL;
	struct pt_regs *dest_pt_regs = NULL;
	int64_t offset = 0;
	unsigned char *kernel_reserved_sp = &__kernel_stack_start;
	struct page *pgd_page = NULL;
	pgd_t *pgd_dest = NULL, *pgd_src = NULL;
	
	irq_state = lock_irq_save();

	for (int i = 0; i < TASK_POOL_SIZE; i++) {
		if (task_pool[i].state == EXIT) {
			dest = &task_pool[i];
			break;
		}
	}

	if (dest == NULL) {
		printf("\rCannot find an usable task\n");
		src_pt_regs->regs[0] = (uint64_t) -1;
		goto Failed;
	}

	//clone cpu_context
	memcpy(&dest->cpu_context, &src->cpu_context, sizeof(struct cpu_context));

	dest->epoch = DEFAULT_EPOCH;
	dest->need_reschedule = 0;
	dest->priority = src->priority;
	dest->runnable_task_parent = NULL;
	dest->state = RUNNING;
	INIT_LIST_HEAD(&dest->list);

	//kernel sp
	dest_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * dest->task_id;
	src_sp = kernel_reserved_sp + KERNEL_STACK_SIZE * src->task_id;
	memcpy(dest_sp, src_sp, KERNEL_STACK_SIZE);

	
	offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)src_pt_regs;
	//printf("\rpt_regs offset:%lld\n", offset);
	dest_pt_regs = (struct pt_regs *)((unsigned char *) dest->reserved_kernel_sp - offset);

	dest->cpu_context.pc = lr_address;
	offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)sp_address;
	//printf("\rkernel sp offset:%lld\n", offset);
	dest->cpu_context.sp = (uint64_t)((unsigned char *)dest->reserved_kernel_sp - offset);

	if (src->cpu_context.fp) {
		offset = (unsigned char *)src->reserved_kernel_sp - (unsigned char *)src->cpu_context.fp;
		dest->cpu_context.fp = (uint64_t)((unsigned char *)dest->reserved_kernel_sp - offset);
	}

	//copy pgd
	if (src->user_pgd_page != NULL) {
		pgd_page = page_alloc();
		if (pgd_page == NULL) {
			goto Failed;
		}

		pgd_dest = (pgd_t *) PHYS_TO_KERNEL_VIRT(PAGE_FRAME_NUM_TO_PHYS(pgd_page->page_num));
		pgd_src = (pgd_t *) PHYS_TO_KERNEL_VIRT(PAGE_FRAME_NUM_TO_PHYS(src->user_pgd_page->page_num));
		
		if (mmu_copy_pgd(pgd_dest, pgd_src)) {
			goto Failed;
		}

		dest->user_pgd_page = pgd_page;

	} else {
		dest->user_pgd_page = NULL;
	}
	
	scheduler_add_task_to_queue(dest, RUNNABLE_TASK_CURRENT);

	dest_pt_regs->regs[0] = 0;
	src_pt_regs->regs[0] = dest->task_id;
	
	lock_irq_restore(irq_state);
	return;

Failed:
	if (dest != NULL) {
		dest->state = EXIT;
		dest->user_pgd_page = NULL;
	}

	lock_irq_restore(irq_state);
}

int task_get_cur_task_id()
{
	struct task *cur = current;
	return cur->task_id;
}
