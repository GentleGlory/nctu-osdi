#include "image_loader.h"
#include "uart0.h"
#include "string.h"
#include "mailbox.h"
#include "shell.h"

extern unsigned char __bootloader_size;
extern unsigned char __image_start, __image_end;
extern unsigned char __rel_dyn_start, __rel_dyn_end, __rel_dyn_size;

extern void __jump_to_start_loading_kernel(uint64_t start_addr, uint32_t size, 
				uint32_t checksum, uint64_t new_bl_addr);

static uint32_t get_u32_little_endian()
{
	unsigned char data[sizeof(uint32_t)] = {0};
	int cnt = 0;

	while (cnt < sizeof(uint32_t)) {
		data[cnt++] = (unsigned char)uart0_getraw();
	}

	return (uint32_t)data[0] |
			((uint32_t)data[1] << 8) |
			((uint32_t)data[2] << 16) |
			((uint32_t)data[3] << 24);
}

static int32_t get_int_little_endian()
{
	unsigned char data[sizeof(int32_t)] = {0};
	int cnt = 0;

	while (cnt < sizeof(int32_t)) {
		data[cnt++] = (unsigned char)uart0_getraw();
	}

	return (int32_t)data[0] |
			((int32_t)data[1] << 8) |
			((int32_t)data[2] << 16) |
			((int32_t)data[3] << 24);
}

static int check_overlap(uint64_t kernel_addr, int64_t kernel_size)
{
	uint64_t bootloader_start_addr = (uint64_t) &__image_start - DEFAULT_STACK_SIZE;
	uint64_t bootloader_end_addr = (uint64_t) &__image_end;
	uint64_t kernel_end_addr = kernel_addr + kernel_size - 1;

	return bootloader_start_addr <= kernel_end_addr &&
			kernel_addr <= bootloader_end_addr;
}

static int calculate_new_bootloader_addr(
	uint64_t kernel_addr, uint32_t kernel_size, 
	uint64_t *bootloader_addr)
{
	int ret = 0;
	uint64_t new_addr = kernel_addr + kernel_size + DEFAULT_STACK_SIZE;
	uint32_t arm_mem_base_addr = 0, arm_mem_size = 0;
	uint64_t arm_mem_end_addr = 0;
	uint64_t bootloader_size = (uint64_t)&__bootloader_size;
	
	new_addr = (new_addr + 0xFFFF) & ~(0xFFFF); //Alignment

	mailbox_get_arm_mem_info(&arm_mem_base_addr, &arm_mem_size);
	arm_mem_end_addr = arm_mem_base_addr + arm_mem_size - 1;

	if (new_addr + bootloader_size - 1 > arm_mem_end_addr) {
		new_addr = kernel_addr - bootloader_size;
		new_addr = new_addr & ~(0xFFFF); //Alignment

		if (new_addr < DEFAULT_STACK_SIZE ||
			new_addr - DEFAULT_STACK_SIZE < arm_mem_base_addr) {
			ret = -1;
		}
	}

	*bootloader_addr = new_addr;
	return ret;
}

static void relocate_bootloader(uint64_t new_addr)
{
	uint64_t bootloader_start_addr = (uint64_t)&__image_start;
	int64_t offset = (int64_t)new_addr - (int64_t)bootloader_start_addr;
	uint64_t bootloader_size = (uint64_t)&__bootloader_size;
	unsigned char *new_bl = (unsigned char *)new_addr;
	unsigned char *bl = (unsigned char *)&__image_start;
	uint64_t rel_size = (uint64_t)&__rel_dyn_size / sizeof(struct _rel_info);
	struct _rel_info *rel_info = (struct _rel_info *)((uint64_t)&__rel_dyn_start + offset);

	printf("\r relocate bootloader start:%lld, offset:%lld, size:%lld\n",
			bootloader_start_addr, offset, bootloader_size);
	
	for (int i = 0; i < bootloader_size; i++) {
		*new_bl++ = *bl++;
	}

	for (int i = 0; i < rel_size; i++) {
		uint64_t type = rel_info[i].info &0xffffffff;
		if (type == R_AARCH64_RELATIVE) {
			rel_info[i].offset += offset;
			rel_info[i].addend += offset;
			long long* fix = (long long*)(rel_info[i].offset);
			(*fix) = rel_info[i].addend;
		}
	}
}

static void clear_old_bl(uint64_t old_bl_addr, uint64_t old_bl_size)
{
	unsigned char *bl = (unsigned char *)old_bl_addr;
	if (old_bl_size == 0)
		return;

	for (int i = 0; i < old_bl_size; i++) {
		*bl++ = 0;
	}
}

void start_loading_kernel(uint64_t start_addr, 
	uint64_t size, int64_t checksum, uint64_t old_bl_addr, uint64_t old_bl_size)
{
	char * kernel = (char *)start_addr;
	int64_t img_checksum = 0;
	
	clear_old_bl(old_bl_addr, old_bl_size);
	printf("\rPlease send kernel image from UART now...\n");

	for (int i = 0; i < size; i++) {
		char b = uart0_getraw();
		*(kernel + i) = b;
		img_checksum += (int)b;
	}

	if (img_checksum != checksum) {
		printf("\rWrong Image checksum, %lld vs %lld\n", img_checksum, checksum);
		printf("\rload_image Error\n");
		shell_main();
	} else {
		void (*start_os)(void) = (void *) kernel;
		start_os();
	}

	//Should not be here.
	while (1) {
		
	}
}

void load_image()
{
	uint64_t address = IMAGE_LOADER_DEFAULT_ADDRESS;
	uint64_t img_size = 0;
	int64_t checksum = 0;
	
	printf("\rStart Loading kernel image...\n");
	printf("\rPlease input kernel load address (default: 0x%llx):\n",
			IMAGE_LOADER_DEFAULT_ADDRESS);
	
	if (simple_scanf("%llx", &address) != 1) {
		address = IMAGE_LOADER_DEFAULT_ADDRESS;
	}

	printf("\rPlease send kernel size and checksum\n");

	//First 4 bytes is image size;
	img_size = get_u32_little_endian();
	//Second 4 bytes is checksum;
	checksum = get_int_little_endian();

	printf("\rKernel Image Size:%llu, checksum: %lld , load address: 0x%llx\n",
			img_size, checksum, address);
	
	if (check_overlap(address, img_size)) {
		uint64_t bootloader_new_addr = 0;
		if (calculate_new_bootloader_addr(
			address, img_size, &bootloader_new_addr) == 0) {

			relocate_bootloader(bootloader_new_addr);

			__jump_to_start_loading_kernel(address, img_size, 
				checksum, bootloader_new_addr);
		} else {
			printf("\rkernel over lap the bootloader and cannot find a new address to relocate bootloader\n");
			printf("\rPlease set a new start address for the kernel or decrease the kernel size\n");
			printf("\rload_image Error\n");
			return;
		}
	} else {
		start_loading_kernel(address, img_size, checksum, 0 , 0);
	}
}