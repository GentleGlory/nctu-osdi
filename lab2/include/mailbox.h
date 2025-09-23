#ifndef _MAILBOX_H
#define _MAILBOX_H

#include "core.h"
#include "fb.h"

#define MAILBOX_BASE	(MMIO_BASE + 0x0000b880)

#define MAILBOX_READ_REG			MAILBOX_BASE
#define MAILBOX_READ_STATUS_REG		(MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE_REG			(MAILBOX_BASE + 0x20)
#define MAILBOX_WRITE_STATUS_REG	(MAILBOX_BASE + 0x38)

#define MAILBOX_EMPTY			0x40000000
#define MAILBOX_FULL			0x80000000

//Tags
#define MAILBOX_GET_BOARD_REVISION		0x00010002
#define MAILBOX_GET_ARM_MEMORY_BASE		0x00010005
#define MAILBOX_GET_VC_MEMORY_BASE		0x00010006
#define MAILBOX_GET_CLK_STATE			0x00030001
#define MAILBOX_SET_CLK_STATE			0x00038001
#define MAILBOX_GET_CLK_RATE			0x00030002
#define MAILBOX_SET_CLK_RATE			0x00038002

#define MAILBOX_FB_ALLOC_BUFFER			0x00040001
#define MAILBOX_FB_SET_PHY_WH			0x00048003
#define MAILBOX_FB_SET_VIRTUAL_WH		0x00048004
#define MAILBOX_FB_SET_DEPTH			0x00048005
#define MAILBOX_FB_SET_RGB_ORDER		0x00048006
#define MAILBOX_FB_GET_PITCH			0x00040008
#define MAILBOX_FB_SET_VIRTUAL_OFFSET	0x00048009

#define MAILBOX_PROPERTY_CHANNEL		8

#define MAILBOX_REQUEST_CODE		0x00000000
#define MAILBOX_REQUEST_SUCCEED		0x80000000
#define MAILBOX_REQUEST_FAILED		0x80000001
#define MAILBOX_TAG_REQUEST_CODE	0x00000000
#define MAILBOX_END_TAG				0x00000000

#define MAILBOX_CLK_ID_UART			0x000000002

uint32_t mailbox_get_board_reversion();
void mailbox_get_vc_mem_info(uint32_t *address, uint32_t *size);
void mailbox_get_arm_mem_info(uint32_t *address, uint32_t *size);
void mailbox_get_clock_state(uint32_t clock_id, int *is_on, int *exist);
int mailbox_set_clock_state(uint32_t clock_id, int on);
unsigned int mailbox_get_clock_rate(uint32_t clock_id);
void mailbox_set_clock_rate(uint32_t clock_id, uint32_t clock_rate, uint32_t skip_turbo);
int mailbox_init_fb_info(struct fb_info *fb_info);

#endif