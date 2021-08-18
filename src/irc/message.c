#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <irc/message.h>

#include <logger.h>

const char **irc_message_split_arguments(char *data)
{
	char *const start = data;
	const char **arguments;
	unsigned count;

	count = 0;
	while (*data != '\0')
	{
		while (*data && *data != IRC_MESSAGE_DELIM)
			data++;
		if (*data != '\0')
			*data++ = '\0';
		while (*data == IRC_MESSAGE_DELIM)
			data++;
		count++;
		if (*data == IRC_MESSAGE_LAST_ARG_PREFIX)
		{
			count++;
			break;
		}
	}

	arguments = malloc(sizeof(*arguments) * (count + 1));
	if (arguments)
	{
		data = start;
		for (unsigned i = 0; i < count; i++)
		{
			arguments[i] = data;
			while (*data != '\0')
				data++;
			data++;
		}
		arguments[count] = NULL;
	}

	return arguments;
}

void irc_msg_parse(irc_msg *message, char *data)
{
	debug("Parsing message '%s'...\n", data);
	if (*data == IRC_MESSAGE_PREFIX_PREFIX)
	{
		data++;
		message->prefix = data;
	}
	// TODO: Maybe handle errors here

	while (*data != '\0' && *data != IRC_MESSAGE_DELIM)
		data++;
	if (*data != '\0')
		*data++ = '\0';
	while (*data && *data == IRC_MESSAGE_DELIM)
		data++;
	message->id = data;
	while (*data && *data != IRC_MESSAGE_DELIM)
		data++;
	if (*data != '\0')
		*data++ = '\0';
	message->arguments = irc_message_split_arguments(data);
	if (!message->arguments)
		irc_msg_clr(message);
}

void irc_msg_clr(irc_msg *message)
{
	free(message->arguments);
	bzero(message, sizeof(*message));
}
