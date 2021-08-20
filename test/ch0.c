#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include <irc/connection.h>

#include <logger.h>

static void on_welcome(irc_con *const connection, const irc_msg *const message)
{
	printf("Received welcome '%s' on fd %d!\n", message->id, connection->fd);
	irc_buff_append(&connection->write, "PRIVMSG Candy :!ep1" IRC_MESSAGE_SUFFIX);
}

static void on_message(irc_con *const connection, const irc_msg *const message)
{
	char num_buff[12];
	int a;
	int b;
	if (message->arguments != NULL && message->arguments[0] != NULL && message->arguments[1] != NULL && strcmp("Candy!Candy@root-me.org", message->prefix) == 0)
	{
		printf("Received message '%s' on fd %d!\n", message->arguments[1], connection->fd);
		if (sscanf(message->arguments[1], "%d / %d", &a, &b) == 2)
		{
			printf("Parsed operants a: %d and b: %d\n", a, b);
			printf("Solution is %.2f\n", sqrt(a) * b);
			snprintf(num_buff, sizeof(num_buff) - 1, "%.2f", sqrtf(a) * b);
			irc_buff_append(&connection->write, "PRIVMSG Candy :!ep1 -rep ");
			irc_buff_append(&connection->write, num_buff);
			irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);
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
