#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <irc/address.h>
#include <irc/connection.h>
#include <irc/message.h>

#include <logger.h>

SSL_CTX *irc_create_ssl_context()
{
	const SSL_METHOD *const method = SSLv23_client_method();
	SSL_CTX *const ctx = SSL_CTX_new(method);

	if (!ctx)
	{
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
	}

	return ctx;
}

int irc_con_init_ssl(irc_con *const connection, SSL_CTX *const ssl_context)
{
	int err;
	debug("Creating an ssl instance...\n");

	connection->ssl = SSL_new(ssl_context);
	if (connection->ssl != NULL)
		err = -(SSL_set_fd(connection->ssl, connection->fd) == 0);
	else
	{
		close(connection->fd);
		connection->fd = -1;
		err = -2;
	}
	return err;
}

int irc_con_init(irc_con *const connection, SSL_CTX *const ssl_context)
{
	int err = 0;
	int flags;

	debug("Initializing socket...\n");
	bzero(connection, sizeof(*connection));
	connection->fd = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (connection->fd == -1)
	{
		perror("socket");
		err = -1;
	}
	else
	{
		flags = fcntl(connection->fd, F_GETFL);
		if (flags == -1 || fcntl(connection->fd, F_SETFL, O_NONBLOCK) != 0)
		{
			perror("fcntl");
			err = -2;
		}
		else
		{
			connection->handlers = hash_map_new(IRC_HANDLER_MAX);
			if (connection->handlers == NULL)
				err = -3;
			else if (ssl_context != NULL)
				err = irc_con_init_ssl(connection, ssl_context);
		}
		if (err != 0)
		{
			close(connection->fd);
			connection->fd = -1;
		}
	}

	return err;
	/* This is equivalent right now
  connection->fd = fd;
  connection->msg_handlers = NULL;
  connection->ssl_context = NULL;
  irc_buff_init(&connection->read);
  irc_buff_init(&connection->write);
  irc_handlers_init(connection->handlers); */
}

void irc_con_destroy(irc_con *const connection)
{
	irc_close(connection);
	hash_map_clr(&connection->handlers, (void *(*)(void *)) & irc_handler_clr);
}

irc_con *irc_con_new(SSL_CTX *const ssl_context)
{
	irc_con *connection = malloc(sizeof(*connection));

	if (connection)
	{
		if (irc_con_init(connection, ssl_context) != 0)
		{
			free(connection);
			connection = NULL;
		}
	}
	else
		perror("malloc");

	return connection;
}

void irc_con_del(irc_con **const connection)
{
	irc_con_destroy(*connection);
	free(*connection);
	*connection = NULL;
}

int irc_connect(irc_con *const connection, const char *hostname, const char *service)
{
	int ret;

	printf("Connecting to %s:%s...\n", hostname, service);

	ret = irc_getaddr(&connection->server_address, hostname, service);
	if (ret == 0)
	{
		ret = connect(connection->fd, connection->server_address->ai_addr, connection->server_address->ai_addrlen);
		if (ret != 0 && errno != EINPROGRESS)
			perror("connect");
		else if (connection->ssl != NULL && SSL_connect(connection->ssl) != 1)
		{
			ERR_print_errors_fp(stderr);
			ret = -2;
		}
	}
	return ret;
}

void irc_authenticate(irc_con *const connection, const char *nickname, const char *username, const char *realname, const char *pass)
{
	// TODO: Handle allocation errors

	irc_buff_append(&connection->write, "NICK ");
	irc_buff_append(&connection->write, nickname);
	irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);

	irc_buff_append(&connection->write, "USER ");
	irc_buff_append(&connection->write, username);
	irc_buff_append(&connection->write, " . . :");
	irc_buff_append(&connection->write, realname);
	irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);

	if (pass != NULL && *pass != '\0')
	{
		irc_buff_append(&connection->write, "PASS ");
		irc_buff_append(&connection->write, pass);
		irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);
	}
	debug("Authenticate: write buffer: %s\n", connection->write.buffer);
}

void irc_close(irc_con *const connection)
{
	if (connection->fd != -1)
	{
		debug("Closing fd %d...\n", connection->fd);
		if (close(connection->fd))
			perror("close");

		connection->fd = -1;
	}
	SSL_free(connection->ssl);
	freeaddrinfo(connection->server_address);
	irc_buff_clr(&connection->read);
	irc_buff_clr(&connection->write);
	connection->ssl = NULL;
	connection->server_address = NULL;
}

/*
void irc_handle_message(irc_con *connection)
{
	// :[PREFIX] [CODE] [a] [b] [c] :[d e f]



	// TODO: Parse and dispatch message to handlers
}
*/

int irc_read(irc_con *connection)
{
	irc_msg message;
	ssize_t ret;
	size_t remaining;
	char *end;

	ret = 1;
	remaining = 0;

	if (connection->read.size != 0)
		remaining = connection->read.size - connection->read.length - 1;
	debug("Reading from fd %d, %zu bytes remaining...\n", connection->fd, remaining);

	while (ret > 0)
	{
		if (remaining < IRC_READ_CHUNK_SIZE)
		{
			ret = irc_buff_resize(&connection->read, connection->read.size == 0 ? IRC_READ_CHUNK_SIZE : connection->read.size * 2);

			if (ret == -1)
				break;

			remaining = ret - 1;
		}

		if (connection->ssl != NULL)
			ret = SSL_read(connection->ssl, connection->read.buffer + connection->read.length, remaining);
		else
			ret = recv(connection->fd, connection->read.buffer + connection->read.length, remaining, 0);

		debug("Read returned %zd!\n", ret);

		if (ret > 0)
		{
			connection->read.length += ret;
			remaining -= ret;

			connection->read.buffer[connection->read.length] = '\0';

			debug("Read %zd bytes from fd %d!\n", ret, connection->fd);

			end = strstr(connection->read.buffer + connection->read.length - ret, IRC_MESSAGE_SUFFIX);
			while (end != NULL)
			{
				*end = '\0';
				irc_msg_parse(&message, connection->read.buffer);
				if (message.id != NULL)
					irc_handler_dispatch(connection, &message);
				debug("Message: prefix: '%s', id: '%s'\n", message.prefix, message.id);
				irc_msg_clr(&message);
				irc_buff_rotate(&connection->read, end - connection->read.buffer + sizeof(IRC_MESSAGE_SUFFIX) - 1);
				end = strstr(connection->read.buffer, IRC_MESSAGE_SUFFIX);
			}
		}
		else if (ret == 0)
			printf("Connection on fd %d has been closed by the host.\n", connection->fd);
		else
		{
			if (errno == EWOULDBLOCK)
				ret = 1;
			else
				perror("recv");
			break;
		}
	}

	return ret;
}

int irc_write(irc_con *connection)
{
	ssize_t ret;

	ret = connection->write.length;

	debug("Attempting to write %zd bytes...\n", ret);

	while (ret > 0 && connection->write.length != 0)
	{
		if (connection->ssl != NULL)
			ret = SSL_write(connection->ssl, connection->write.buffer, connection->write.length);
		else
			ret = send(connection->fd, connection->write.buffer, connection->write.length, 0);
		debug("Sent %zd bytes on fd %d!\n", ret, connection->fd);
		if (ret > 0)
		{
			debug("Sent '%.*s'\n", (int)ret, connection->write.buffer);

			irc_buff_rotate(&connection->write, ret);
		}
	}

	return ret;
}

void ping_handler(irc_con *const connection, const irc_msg *const message)
{
	irc_buff_append(&connection->write, "PONG ");
	if (message->arguments && message->arguments[0])
	{
		debug("Sending PONG with argument %s...\n", message->arguments[0]);
		irc_buff_append(&connection->write, message->arguments[0]);
	}
	else
		debug("Sending PONG...\n");
	irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);
}

int irc_listen(irc_con *connections, unsigned count)
{
	struct pollfd fds[IRC_CONNECTION_MAX];
	unsigned i;
	int ret;

	if (count > IRC_CONNECTION_MAX)
		return -1;
	if (count == 0)
		return 0;

	for (i = 0; i < count; i++)
	{
		fds[i].fd = connections[i].fd;
		fds[i].events = POLLIN;
		irc_on(connections + i, "PING", &ping_handler);
		debug("Write length: %zu\n", connections[i].write.length);
		if (connections[i].write.length != 0)
			fds[i]
				.events |= POLLOUT;
	}

	while (count)
	{
		debug("Polling %u connections (timeout in %d seconds)...\n", count, IRC_CONNECTION_TIMEOUT / 1000);
		ret = poll(fds, count, IRC_CONNECTION_TIMEOUT);

		if (ret > 0)
		{
			for (i = 0; i < count; i++)
			{
				if ((fds[i].revents & POLLIN))
				{
					ret = irc_read(&connections[i]);
					debug("irc_read: ret: %d\n", ret);
					if (ret <= 0)
					{
						fds[i].fd = -1;
						irc_close(&connections[i]);
					}
					if (connections[i].write.length != 0)
					{
						fds[i].events |= POLLOUT;
					}
					debug("Setting POLLOUT %s on fd %d!\n", fds[i].events & POLLOUT ? "on" : "off", fds[i].fd);
				}
				if (fds[i].revents & POLLOUT)
				{
					irc_write(&connections[i]);
					if (connections[i].write.length == 0)
					{
						fds[i].events &= ~POLLOUT;
					}
					debug("Setting POLLOUT %s on fd %d!\n", fds[i].events & POLLOUT ? "on" : "off", fds[i].fd);
				}
				if (fds[i].revents & POLLHUP)
				{
					fds[i].fd = -1;
					irc_close(&connections[i]);
				}
			}
			// TODO: Handle other errors
		}
		else if (ret == 0)
		{
			printf("Poll timed out!\n");
			break;
		}
		else
		{
			perror("poll");
			break;
		}
	}
	for (i = 0; i < count; i++)
		irc_close(&connections[i]);
	return ret;
}
