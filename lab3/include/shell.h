#ifndef _SHELL_H
#define _SHELL_H

#define SHELL_PREFIX		"#"
#define SHELL_BUFFER_LEN	256
enum ANSI_ESC {
	ANSI_ESC_UNKNOWN,
	ANSI_ESC_CURSOR_FORWARD,
	ANSI_ESC_CURSOR_BACKWARD,
	ANSI_ESC_DELETE
};

typedef void (*cmd_fn)(void);

struct shell_cmd {
	const char* cmd;
	const char* description;
	cmd_fn	cmd_fn_ptr;
};

void shell_main();

#endif