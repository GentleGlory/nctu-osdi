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

void mailbox_get_clock_state(uint32_t clock_id, int *is_on, int *exist)
{
	mailbox_buffer[0] = 8 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_GET_CLK_STATE;
	mailbox_buffer[3] = 8;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = clock_id;
	mailbox_buffer[6] = 0;
	mailbox_buffer[7] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	(*is_on) = mailbox_buffer[6] & BIT(0);
	(*exist) = mailbox_buffer[6] & BIT(1);
}

int mailbox_set_clock_state(uint32_t clock_id, int on)
{
	mailbox_buffer[0] = 8 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_SET_CLK_STATE;
	mailbox_buffer[3] = 8;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = clock_id;
	mailbox_buffer[6] = (on ? BIT(0) : 0 ) | BIT(1);
	mailbox_buffer[7] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	if ((on != 0 && (mailbox_buffer[6] & BIT(0))) || 
		(on == 0 && !(mailbox_buffer[6] & BIT(0)))) {
		return 1;
	}
	return 0;
}

unsigned int mailbox_get_clock_rate(uint32_t clock_id) 
{
	mailbox_buffer[0] = 8 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_GET_CLK_RATE;
	mailbox_buffer[3] = 8;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = clock_id;
	mailbox_buffer[6] = 0;
	mailbox_buffer[7] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	return mailbox_buffer[6];
} 

void mailbox_set_clock_rate(uint32_t clock_id, uint32_t clock_rate, uint32_t skip_turbo)
{
	mailbox_buffer[0] = 9 * 4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	mailbox_buffer[2] = MAILBOX_SET_CLK_RATE;
	mailbox_buffer[3] = 12; 
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = clock_id;
	mailbox_buffer[6] = clock_rate;
	mailbox_buffer[7] = skip_turbo;
	mailbox_buffer[8] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);
}

int mailbox_init_fb_info(struct fb_info *fb_info)
{
	mailbox_buffer[0] = 35*4;
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	//set phy wh
	mailbox_buffer[2] = MAILBOX_FB_SET_PHY_WH;
	mailbox_buffer[3] = 8;
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.width
	mailbox_buffer[5] = fb_info->width;
	//FrameBufferInfo.height
	mailbox_buffer[6] = fb_info->height;

	//set virt wh
	mailbox_buffer[7] = MAILBOX_FB_SET_VIRTUAL_WH;
	mailbox_buffer[8] = 8;
	mailbox_buffer[9] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.virtual_width
	mailbox_buffer[10] = fb_info->width;
	//FrameBufferInfo.virtual_height
	mailbox_buffer[11] = fb_info->height;

	//set virt offset
	mailbox_buffer[12] = MAILBOX_FB_SET_VIRTUAL_OFFSET;
	mailbox_buffer[13] = 8;
	mailbox_buffer[14] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.x_offset
	mailbox_buffer[15] = 0;
	//FrameBufferInfo.y.offset
	mailbox_buffer[16] = 0;

	//set depth
	mailbox_buffer[17] = MAILBOX_FB_SET_DEPTH;
	mailbox_buffer[18] = 4;
	mailbox_buffer[19] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.depth
	mailbox_buffer[20] = fb_info->depth;

	//set pixel order
	mailbox_buffer[21] = MAILBOX_FB_SET_RGB_ORDER;
	mailbox_buffer[22] = 4;
	mailbox_buffer[23] = MAILBOX_TAG_REQUEST_CODE;
	//RGB, not BGR preferably
	mailbox_buffer[24] = fb_info->is_rgb;

	//get framebuffer, gets alignment on request
	mailbox_buffer[25] = MAILBOX_FB_ALLOC_BUFFER;
	mailbox_buffer[26] = 8;
	mailbox_buffer[27] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.pointer
	mailbox_buffer[28] = fb_info->alignment;
	//FrameBufferInfo.size
	mailbox_buffer[29] = 0;

	//get pitch
	mailbox_buffer[30] = MAILBOX_FB_GET_PITCH;
	mailbox_buffer[31] = 4;
	mailbox_buffer[32] = MAILBOX_TAG_REQUEST_CODE;
	//FrameBufferInfo.pitch
	mailbox_buffer[33] = 0;

	mailbox_buffer[34] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer);

	if (mailbox_buffer[20] == 32 && mailbox_buffer[28] != 0) {
		//convert GPU address to ARM address
		mailbox_buffer[28] &= 0x3FFFFFFF;
		//get actual physical width
		fb_info->width = mailbox_buffer[5];
		//get actual physical height
		fb_info->height = mailbox_buffer[6];
		//get number of bytes per line	
		fb_info->pitch = mailbox_buffer[33];
		//get the actual channel order
		fb_info->is_rgb = mailbox_buffer[24];
		fb_info->buffer = (void*)((unsigned long)mailbox_buffer[28]);
		fb_info->size = mailbox_buffer[29];
		return 0;
	}

	return -1;
}

void mailbox_get_arm_mem_info(uint32_t *address, uint32_t *size)
{
	mailbox_buffer[0] = 8 * 4; // buffer size in bytes
	mailbox_buffer[1] = MAILBOX_REQUEST_CODE;

	// tags begin
	mailbox_buffer[2] = MAILBOX_GET_ARM_MEMORY_BASE; // tag identifier
	mailbox_buffer[3] = 8; // maximum of request and response value buffer's length.
	mailbox_buffer[4] = MAILBOX_TAG_REQUEST_CODE;
	mailbox_buffer[5] = 0; // value buffer
	mailbox_buffer[6] = 0; // value buffer
	// tags end
	mailbox_buffer[7] = MAILBOX_END_TAG;

	mailbox_call(mailbox_buffer); // message passing procedure call, you should implement it following the 6 steps provided above.

	*address = mailbox_buffer[5];
	*size = mailbox_buffer[6];
}