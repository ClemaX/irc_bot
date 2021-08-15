#include <stdbool.h>

#include <irc/connection.h>

#include <logger.h>

void on_welcome(irc_con *const connection, const irc_msg *const message)
{
	printf("Received welcome '%s' on fd %d!\n", message->id, connection->fd);
}

void on_message(irc_con *const connection, const irc_msg *const message)
{
	printf("Received message '%s' on fd %d!\n", message->id, connection->fd);
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
		irc_connect(connection, "irc.root-me.org", "6667");
		irc_authenticate(connection, "chamada", "ClemaX", "");
		irc_on_command(connection, "001", &on_welcome);
		irc_on_command(connection, "PRIVMSG", &on_message);
		irc_listen(connection, 1);
		irc_con_del(&connection);
	}
	// }

	// SSL_CTX_free(ssl_context);
	// ssl_context = NULL;

	return 0;
}
