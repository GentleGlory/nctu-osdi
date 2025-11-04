#include "shell.h"
#include "core.h"
#include "string.h"
#include "time.h"
#include "power.h"
#include "mailbox.h"
#include "timer.h"
#include "irq.h"
#include "system_call.h"
#include "delay.h"

#ifdef BOOTLOADER
#include "image_loader.h"
#include "uart.h"

	#define shell_getc()		uart_getc()
	#define shell_putc(x)		uart_putc(x)
	#define uprintf			printf
	#define udelay(x)
#else
static char shell_getc()
{
	size_t size;
	char c;

	while ((size = system_call_uart_read(&c, 1)) == 0) {
		udelay(10);
	}

	return c;
}

static void shell_putc(char c)
{
	system_call_uart_write(&c, 1);
}
#endif

static int cmd_idx = 0;
static int cursor_idx = 0;
static char cmd_buffer[SHELL_BUFFER_LEN] = {0};

static void hello_world_cmd(void);
static void help_cmd(void);
static void read_text_cmd(void);
static void timestamp_cmd(void);
static void reboot_cmd(void);
static void exc_cmd();
static void irq_cmd();

#ifdef BOOTLOADER
static void loadimg_cmd(void); 
#endif

static enum ANSI_ESC decode_csi_key();
static enum ANSI_ESC decode_ansi_escape();

static const struct shell_cmd shell_cmds[] = {
	{	.cmd = "hello",
		.description = "Print \"Hello World!\" and return.",
		.cmd_fn_ptr = hello_world_cmd,
	},
	{	.cmd = "help",
		.description = "Help",
		.cmd_fn_ptr = help_cmd,
	},
	{	.cmd = "read_file",
		.description = "Read text file contents from terminal.",
		.cmd_fn_ptr = read_text_cmd,
	},
	{	.cmd = "timestamp",
		.description = "Get timestamp.",
		.cmd_fn_ptr = timestamp_cmd,
	},
	{	.cmd = "reboot",
		.description = "Reboot the system.",
		.cmd_fn_ptr = reboot_cmd,
	},
	{	.cmd = "exc",
		.description = "Exception test.",
		.cmd_fn_ptr = exc_cmd,
	},	
	{	.cmd = "irq",
		.description = "Enable timer",
		.cmd_fn_ptr = irq_cmd,
	},	
#ifdef BOOTLOADER
	{
		.cmd = "loadimg",
		.description = "Load kernel from uart.",
		.cmd_fn_ptr = loadimg_cmd,
	},
#endif
	{},
};

static void hello_world_cmd(void)
{
	uprintf("\rHello World!\n");
}

static void help_cmd(void)
{
	const struct shell_cmd* cmd = shell_cmds;

	for (; cmd->cmd != NULL; cmd ++) {
		uprintf("\r");
		uprintf(cmd->cmd);
		uprintf(": ");
		uprintf(cmd->description);
		uprintf("\n");
	}
}

static void read_text_cmd(void)
{
	char c;

	uprintf("\rReady to read text file.\n");
	uprintf("\rSending ctrl+c will stop the transmission.\n");

	while (1) {
		c = shell_getc();
		//ctrl+c 
		if (c == 3) {
			return;
		}
			
		shell_putc(c);
	}
}

static void timestamp_cmd(void)
{
	system_call_print_timestamp();
	
}

static void reboot_cmd(void)
{
	power_reset(10);
}

static void exc_cmd()
{
	system_call_test();
}

static void irq_cmd()
{
	syetem_call_irq_test();
}

#ifdef BOOTLOADER
static void loadimg_cmd(void)
{
	load_image();
}
#endif

static enum ANSI_ESC decode_csi_key() 
{
	char c = shell_getc();
	if (c == 'C') {
		return ANSI_ESC_CURSOR_FORWARD;
	} else if (c == 'D') {
		return ANSI_ESC_CURSOR_BACKWARD;
	} else if (c == '3') {
		c = shell_getc();
		if (c == '~') {
			return ANSI_ESC_DELETE;
		}
	}
	return ANSI_ESC_UNKNOWN;
}

static enum ANSI_ESC decode_ansi_escape() 
{
	char c;
	size_t size = 0;

	while ((size = system_call_uart_read(&c, 1)) == 0) {
		udelay(10);
	}

	if (c == '[') {
		return decode_csi_key();
	}
	return ANSI_ESC_UNKNOWN;
}

static void exec_shell_cmd(const char* cmd_name)
{
	const struct shell_cmd* cmd = shell_cmds;

	for (; cmd->cmd != NULL; cmd ++) {
		if (strcmp(cmd->cmd, cmd_name) == 0) {
			cmd->cmd_fn_ptr();
			return;
		}
	}

	uprintf("\rErr: command %s not found, try <help>\n",cmd_name);
}

static void shell_flush()
{
	size_t size;
	char c;

	while ((size = system_call_uart_read(&c, 1)) != 0) {

	}
}

void shell_main()
{
	char c;
	size_t size = 0;
	
	uprintf("\r# ");
	
	while (1) {
		c = shell_getc();
		if (size == 0)
			udelay(50);
		// \e
		if (c == 27) {
			enum ANSI_ESC key = decode_ansi_escape();
			switch (key) {
			case ANSI_ESC_CURSOR_FORWARD:
				if (cursor_idx < cmd_idx) cursor_idx++;
			break;
			case ANSI_ESC_CURSOR_BACKWARD:
				if (cursor_idx > 0) cursor_idx--;
			break;
			case ANSI_ESC_DELETE:
				// left shift command
				for (int i = cursor_idx; i < cmd_idx; i++) {
					cmd_buffer[i] = cmd_buffer[i + 1];
				}
				if(cmd_idx > 0)
				cmd_buffer[--cmd_idx] = '\0';
			break;
			case ANSI_ESC_UNKNOWN:
				shell_flush();
			break;
			}
		} else if (c == 3) { // CTRL-C
			cmd_buffer[0] = '\0';
			cursor_idx = 0;
			cmd_idx = 0;
			uprintf("\r\e[K# ");
		} else if (c == 8 || c == 127) {// Backspace
			if (cursor_idx > 0) {
				cursor_idx--;
				// left shift command
				for (int i = cursor_idx; i < cmd_idx; i++) {
					cmd_buffer[i] = cmd_buffer[i + 1];
				}
				cmd_buffer[--cmd_idx] = '\0';
			}
		} else if(c == '\n') {
			uprintf("\r\n");
			if (cmd_idx > 0) {
				exec_shell_cmd(cmd_buffer);
			}
			cmd_buffer[0] = '\0';
			cursor_idx = 0;
			cmd_idx = 0;
			uprintf("\r# ");
		} else {
			// right shift command
			if (cursor_idx < cmd_idx) {
				for (int i = cmd_idx; i > cursor_idx; i--) {
					cmd_buffer[i] = cmd_buffer[i - 1];
				}
			}
			cmd_buffer[cursor_idx++] = c;
			cmd_buffer[++cmd_idx] = '\0';
		}
		uprintf("\r# %s \r\e[%dC", cmd_buffer, cursor_idx + 2);
	}
}