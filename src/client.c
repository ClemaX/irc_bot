#include <stdbool.h>
#include <signal.h>

#include <irc/connection.h>

#include <logger.h>

static void on_welcome(irc_con *const connection, const irc_msg *const message)
{
	printf("Received welcome '%s' on fd %d!\n", message->id, connection->fd);
	irc_buff_append(&connection->write, "PRIVMSG Candy :!ep1" IRC_MESSAGE_SUFFIX);
}

static void on_message(irc_con *const connection, const irc_msg *const message)
{
	printf("Received message '");
	for (unsigned i = 0; message->arguments[i] != NULL; i++)
		printf("%s ", message->arguments[i]);
	printf("on fd: %d\n", connection->fd);
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
		irc_connect(connection, "localhost", "6667"); //"chat.freenode.net", "6666"); //
		irc_authenticate(connection, "chamada", "clemax", "");
		irc_listen(connection, 1);
		irc_con_del(&connection);
	}
	// }

	// SSL_CTX_free(ssl_context);
	// ssl_context = NULL;

	return 0;
}
