#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include <irc/connection.h>

#include <base64.h>

#include <logger.h>
#include <zlib.h>

static void on_welcome(irc_con *const connection, const irc_msg *const message)
{
	printf("Received welcome '%s' on fd %d!\n", message->id, connection->fd);
	irc_buff_append(&connection->write, "PRIVMSG Candy :!ep4" IRC_MESSAGE_SUFFIX);
}

static void on_message(irc_con *const connection, const irc_msg *const message)
{
	char *decoded;
	size_t decoded_size;
	char uncompressed[32];
	size_t uncompressed_size = sizeof(uncompressed) - 1;

	if (message->arguments != NULL && message->arguments[0] != NULL && message->arguments[1] != NULL && strcmp("Candy!Candy@root-me.org", message->prefix) == 0)
	{
		printf("Received message '%s' on fd %d!\n", message->arguments[1], connection->fd);

		decoded = base64_decode((const unsigned char*)message->arguments[1], &decoded_size);
		printf("Decoded: '%s'\n", decoded);
		if (decoded != NULL)
		{
			if (uncompress((Bytef*)uncompressed, &uncompressed_size, (Bytef*)decoded, decoded_size) == Z_OK)
			{
				uncompressed[uncompressed_size] = '\0';
				irc_buff_append(&connection->write, "PRIVMSG Candy !ep4 -rep :");
				irc_buff_append(&connection->write, uncompressed);
				irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);
			}
			free(decoded);
		}
	}
}

int main(int ac, const char **av)
{
	(void)ac;
	(void)av;

	SSL_CTX *ssl_context = NULL;
	irc_con *connection;

	// ssl_context = irc_create_ssl_context();
	// if (ssl_context != NULL)
	// {
	connection = irc_con_new(ssl_context);
	if (connection != NULL)
	{
		irc_on(connection, "NOTICE", &on_message);
		irc_on(connection, "001", &on_welcome);
		irc_on(connection, "PRIVMSG", &on_message);
		irc_connect(connection, "irc.root-me.org", "6667");
		irc_authenticate(connection, "chamada", "clemax", "Clement Hamada", "");
		irc_listen(connection, 1);
		irc_con_del(&connection);
	}
	// }

	// SSL_CTX_free(ssl_context);
	// ssl_context = NULL;

	return 0;
}
