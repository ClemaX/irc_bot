#pragma once

#include <irc/ircdef.h>

typedef struct s_irc_message
{
	const char *prefix;
	const char *id;
	const char **arguments;
} irc_msg;

void irc_msg_parse(irc_msg *message, char *data);
void irc_msg_clr(irc_msg *message);
