#include "shell.h"
#include "uart0.h"
#include "core.h"
#include "string.h"
#include "mailbox.h"
#include "image_loader.h"


static int cmd_idx = 0;
static int cursor_idx = 0;
static char cmd_buffer[SHELL_BUFFER_LEN] = {0};

static void help_cmd(void);
static void loadimg_cmd(void); 

static enum ANSI_ESC decode_csi_key();
static enum ANSI_ESC decode_ansi_escape();

static const struct shell_cmd shell_cmds[] = {
	{	.cmd = "help",
		.description = "Help",
		.cmd_fn_ptr = help_cmd,
	},
	{
		.cmd = "loadimg",
		.description = "Load kernel from uart.",
		.cmd_fn_ptr = loadimg_cmd,
	},
	{},
};

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

static void loadimg_cmd(void)
{
	load_image();
}

static enum ANSI_ESC decode_csi_key() 
{
	char c = uart0_getc();
	if (c == 'C') {
		return ANSI_ESC_CURSOR_FORWARD;
	} else if (c == 'D') {
		return ANSI_ESC_CURSOR_BACKWARD;
	} else if (c == '3') {
		c = uart0_getc();
		if (c == '~') {
			return ANSI_ESC_DELETE;
		}
	}
	return ANSI_ESC_UNKNOWN;
}

static enum ANSI_ESC decode_ansi_escape() 
{
	char c = uart0_getc();
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
		c = uart0_getc();
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
				uart0_flush();
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