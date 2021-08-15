#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <irc/buffer.h>

#include <logger.h>

void irc_buff_init(irc_buff *const buffer)
{
	bzero(buffer, sizeof(*buffer));
}

void irc_buff_clr(irc_buff *const buffer)
{
	free(buffer->buffer);
	bzero(buffer, sizeof(*buffer));
}

int irc_buff_resize(irc_buff *const buffer, size_t new_size)
{
	buffer->buffer = realloc(buffer->buffer, new_size);

	if (new_size != 0)
	{
		debug("Resizing buffer to %zu...\n", new_size);
		if (buffer->buffer == NULL)
			return -1;

		if (new_size < buffer->length)
		{
			buffer->length = new_size - 1;
			buffer->buffer[buffer->length] = '\0';
		}
		else if (buffer->size == 0)
			*buffer->buffer = '\0';
	}

	buffer->size = new_size;

	return new_size - buffer->length;
}

int irc_buff_append(irc_buff *const buffer, const char *data)
{
	const size_t remaining = buffer->size == 0 ? 0 : buffer->size - buffer->length - 1;
	const size_t data_length = strlen(data);
	int ret;

	ret = 0;
	if (data_length > remaining)
	{
		ret = irc_buff_resize(buffer, buffer->size + data_length + 1 - remaining);
		if (ret != -1)
		{
			strcpy(buffer->buffer + buffer->length, data);
			buffer->length += data_length;
		}
	}
	return ret;
}

void irc_buff_rotate(irc_buff *const buffer, size_t count)
{
	buffer->length -= count;
	memmove(buffer->buffer, buffer->buffer + count, buffer->length + 1);
}
