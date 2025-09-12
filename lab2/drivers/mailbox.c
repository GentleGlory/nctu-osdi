#include "mailbox.h"


volatile uint32_t __attribute__((aligned(16))) mailbox_buffer[36];

static uint32_t mailbox_read(byte channel)
{
	for (;;) {

		while ((readl(MAILBOX_READ_STATUS_REG) & MAILBOX_EMPTY) != 0 ) {
			asm volatile("nop");
		}

		uint32_t data = readl(MAILBOX_READ_REG);
		byte read_channel = data & 0xf;
		data >>= 4;
		if (read_channel == channel) {
			return data;
		}
	}
}

static void mailbox_write(byte channel, uint32_t data)
{
	while ((readl(MAILBOX_WRITE_STATUS_REG) & MAILBOX_FULL) != 0) {
		asm volatile("nop");
	}

	writel(MAILBOX_WRITE_REG, (data << 4) | channel);
}

static void mailbox_call(volatile uint32_t *buffer)
{
	uint32_t data_addr = ((uint64_t)buffer & (~0xf)) >> 4;
	mailbox_write(MAILBOX_PROPERTY_CHANNEL, data_addr);
	mailbox_read(MAILBOX_PROPERTY_CHANNEL);
}

uint32_t mailbox_get_board_reversion()
{
	mailbox_buffer[0] = 7 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_GET_BOARD_REVISION;
	mailbox_buffer[3] = 4;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = 0;
	mailbox_buffer[6] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	return mailbox_buffer[5];
}

void mailbox_get_vc_mem_info(uint32_t *address, uint32_t *size)
{
	mailbox_buffer[0] = 8 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_GET_VC_MEMORY_BASE;
	mailbox_buffer[3] = 8;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = 0;
	mailbox_buffer[6] = 0;
	mailbox_buffer[7] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	*address = mailbox_buffer[5];
	*size = mailbox_buffer[6];
}