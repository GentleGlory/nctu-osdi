#include "sd.h"
#include "gpio.h"


static inline void sd_delay(unsigned long tick)
{
	while (tick--) {
		asm volatile("nop");
	}
}

static int is_hcs;  // high capcacity(SDHC)

static void pin_setup()
{
	
	gpio_pin_sel_fn(48, GPF_FUNC_0);
	gpio_pin_sel_fn(49, GPF_FUNC_0);
	gpio_pin_sel_fn(50, GPF_FUNC_0);
	gpio_pin_sel_fn(51, GPF_FUNC_0);
	gpio_pin_sel_fn(52, GPF_FUNC_0);
	gpio_pin_sel_fn(53, GPF_FUNC_0);
	
	gpio_pin_set_pud(48, PULL_TYPE_NONE);
	gpio_pin_set_pud(49, PULL_TYPE_NONE);
	gpio_pin_set_pud(50, PULL_TYPE_NONE);
	gpio_pin_set_pud(51, PULL_TYPE_NONE);
	gpio_pin_set_pud(52, PULL_TYPE_NONE);
	gpio_pin_set_pud(53, PULL_TYPE_NONE);
}

static void sdhost_setup()
{
	
	uint32_t tmp;
	
	writel(SDHOST_PWR, 0);
	writel(SDHOST_CMD, 0);
	writel(SDHOST_ARG, 0);
	writel(SDHOST_TOUT, SDHOST_TOUT_DEFAULT);
	writel(SDHOST_CDIV, 0);
	writel(SDHOST_HSTS, SDHOST_HSTS_MASK);
	writel(SDHOST_CFG, 0);
	writel(SDHOST_CNT, 0);
	writel(SDHOST_SIZE, 0);

	tmp = readl(SDHOST_DBG);
	tmp &= ~SDHOST_DBG_MASK;
	tmp |= SDHOST_DBG_FIFO;
	writel(SDHOST_DBG, tmp);
	
	sd_delay(250000);
	writel(SDHOST_PWR, 1);
	sd_delay(250000);
	writel(SDHOST_CFG, SDHOST_CFG_SLOW | SDHOST_CFG_INTBUS | SDHOST_CFG_DATA_EN);
	writel(SDHOST_CDIV, SDHOST_CDIV_DEFAULT);
}

static int wait_sd()
{
	int cnt = 1000000;
	uint32_t cmd;
	
	do {
		if (cnt == 0) {
			return -1;
		}
		cmd = readl(SDHOST_CMD);
		--cnt;
	} while (cmd & SDHOST_NEW_CMD);
	
	return 0;
}

static int sd_cmd(unsigned cmd, unsigned int arg)
{
	
	writel(SDHOST_ARG, arg);
	writel(SDHOST_CMD, cmd | SDHOST_NEW_CMD);
	return wait_sd();
}

static int sdcard_setup()
{
	
	uint32_t tmp;
	sd_cmd(GO_IDLE_STATE | SDHOST_NO_REPONSE, 0);
	sd_cmd(SEND_IF_COND, VOLTAGE_CHECK_PATTERN);

	tmp = readl(SDHOST_RESP0);
	
	if (tmp != VOLTAGE_CHECK_PATTERN) {
		return -1;
	}

	while (1) {
		if (sd_cmd(APP_CMD, 0) == -1) {
			// MMC card or invalid card status
			// currently not support
			continue;
		}
		
		sd_cmd(SD_APP_OP_COND, SDCARD_3_3V | SDCARD_ISHCS);
		
		tmp = readl(SDHOST_RESP0);		
		if (tmp & SDCARD_READY) {
			break;
		}
		sd_delay(1000000);
	}

	is_hcs = tmp & SDCARD_ISHCS;
	sd_cmd(ALL_SEND_CID | SDHOST_LONG_RESPONSE, 0);
	sd_cmd(SEND_RELATIVE_ADDR, 0);
	
	tmp = readl(SDHOST_RESP0);
	sd_cmd(SELECT_CARD, tmp);
	sd_cmd(SET_BLOCKLEN, 512);
	return 0;
}

static int wait_fifo()
{
	int cnt = 1000000;
	uint32_t hsts;
	do {
		if (cnt == 0) {
			return -1;
		}
		hsts = readl(SDHOST_HSTS);
		--cnt;
	} while ((hsts & SDHOST_HSTS_DATA) == 0);
	return 0;
}

static void set_block(int size, int cnt)
{

	writel(SDHOST_SIZE, size);
	writel(SDHOST_CNT, cnt);
}

static void wait_finish()
{
	uint32_t dbg;
	do {
		dbg = readl(SDHOST_DBG);		
	} while ((dbg & SDHOST_DBG_FSM_MASK) != SDHOST_HSTS_DATA);
}

void sd_readblock(int block_idx, void* buf) 
{
	uint32_t* buf_u = (uint32_t *)buf;
	int succ = 0;
	if (!is_hcs) {
		block_idx <<= 9;
	}

	do{
		set_block(512, 1);
		sd_cmd(READ_SINGLE_BLOCK | SDHOST_READ, block_idx);
		for (int i = 0; i < 128; ++i) {
			wait_fifo();
			buf_u[i] = readl(SDHOST_DATA);			
		}

		unsigned int hsts;
		hsts = readl(SDHOST_HSTS);
		if (hsts & SDHOST_HSTS_ERR_MASK) {
			writel(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
			sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
		} else {
			succ = 1;
		}
	} while(!succ);
	wait_finish();
}

void sd_writeblock(int block_idx, void* buf) {
	
	uint32_t *buf_u = (uint32_t *) buf;
	int succ = 0;
	if (!is_hcs) {
		block_idx <<= 9;
	}
	do{
		set_block(512, 1);
		sd_cmd(WRITE_SINGLE_BLOCK | SDHOST_WRITE, block_idx);
		for (int i = 0; i < 128; ++i) {
			wait_fifo();
			writel(SDHOST_DATA, buf_u[i]);
		}
		
		unsigned int hsts;
		hsts = readl(SDHOST_HSTS);
		if (hsts & SDHOST_HSTS_ERR_MASK) {
			writel(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
			sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
		} else {
			succ = 1;
		}
	} while(!succ);
	wait_finish();
}

void sd_init()
{
	pin_setup();
	sdhost_setup();
	sdcard_setup();
}