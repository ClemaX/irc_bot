#pragma once

#include <stddef.h>

#include <irc/ircdef.h>

typedef struct irc_buff_s
{
	char *buffer;
	size_t length;
	size_t size;
} irc_buff;

void irc_buff_init(irc_buff *const buffer);
void irc_buff_clr(irc_buff *const buffer);
int irc_buff_resize(irc_buff *const buffer, size_t new_size);
int irc_buff_append(irc_buff *const buffer, const char *data);
void irc_buff_rotate(irc_buff *const buffer, size_t count);
