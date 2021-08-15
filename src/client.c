#include <stdbool.h>

#include <irc/connection.h>

void on_message(irc_con *const connection, const char *sender, const char *message)
{
	(void)connection;
	(void)sender;
	(void)message;
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
		irc_on_message(connection, &on_message);
		irc_listen(connection, 1);
		irc_con_del(&connection);
	}
	// }

	// SSL_CTX_free(ssl_context);
	// ssl_context = NULL;

	return 0;
}
