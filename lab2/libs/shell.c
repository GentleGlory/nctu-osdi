#include "shell.h"
#include "mini_uart.h"
#include "core.h"
#include "string.h"
#include "time.h"
#include "power.h"
#include "mailbox.h"

static int cmd_idx = 0;
static int cursor_idx = 0;
static char cmd_buffer[SHELL_BUFFER_LEN] = {0};

static void hello_world_cmd(void);
static void help_cmd(void);
static void read_text_cmd(void);
static void timestamp_cmd(void);
static void reboot_cmd(void);

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
	{},
};

static void hello_world_cmd(void)
{
	printf("\rHello World!\n");
}

static void help_cmd(void)
{
	const struct shell_cmd* cmd = shell_cmds;

	for (; cmd->cmd != NULL; cmd ++) {
		printf("\r");
		printf(cmd->cmd);
		printf(": ");
		printf(cmd->description);
		printf("\n");
	}
}

static void read_text_cmd(void)
{
	char c;

	printf("\rReady to read text file.\n");
	printf("\rSending ctrl+c will stop the transmission.\n");

	while (1) {
		c = mini_uart_getc();
		//ctrl+c 
		if (c == 3) {
			return;
		}
			
		mini_uart_putc(c);
	}
}

static void timestamp_cmd(void)
{
	struct rational r = {0, 0};
	
	r = get_time_tick();

	printf("\r[%lld.%lld]\n",r.num, r.den);
}

static void reboot_cmd(void)
{
	reset(10);
}

static enum ANSI_ESC decode_csi_key() 
{
	char c = mini_uart_getc();
	if (c == 'C') {
		return ANSI_ESC_CURSOR_FORWARD;
	} else if (c == 'D') {
		return ANSI_ESC_CURSOR_BACKWARD;
	} else if (c == '3') {
		c = mini_uart_getc();
		if (c == '~') {
			return ANSI_ESC_DELETE;
		}
	}
	return ANSI_ESC_UNKNOWN;
}

static enum ANSI_ESC decode_ansi_escape() 
{
	char c = mini_uart_getc();
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

	printf("\rErr: command %s not found, try <help>\n",cmd_name);
}

void shell_main()
{
	char c;
	
	printf("\r# ");
	
	while (1) {
		c = mini_uart_getc();
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
				mini_uart_flush();
			break;
			}
		} else if (c == 3) { // CTRL-C
			cmd_buffer[0] = '\0';
			cursor_idx = 0;
			cmd_idx = 0;
			printf("\r\e[K# ");
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
			printf("\r\n");
			if (cmd_idx > 0) {
				exec_shell_cmd(cmd_buffer);
			}
			cmd_buffer[0] = '\0';
			cursor_idx = 0;
			cmd_idx = 0;
			printf("\r# ");
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
		printf("\r# %s \r\e[%dC", cmd_buffer, cursor_idx + 2);
	}
}