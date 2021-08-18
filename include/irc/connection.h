#pragma once

#include <stdbool.h>

#include <openssl/ssl.h>

#include <irc/ircdef.h>
#include <irc/handler.h>
#include <irc/buffer.h>
#include <hash_map.h>

typedef struct irc_con_s
{
	int fd;
	irc_buff read;
	irc_buff write;
	hash_map *handlers;
	struct addrinfo *server_address;
	SSL *ssl;
} irc_con;

SSL_CTX *irc_create_ssl_context();

int irc_con_init(irc_con *const connection, SSL_CTX *const context);
void irc_con_destroy(irc_con *const connection);

irc_con *irc_con_new(SSL_CTX *const context);
void irc_con_del(irc_con **const connection);

int irc_connect(irc_con *const connection, const char *hostname, const char *service);
void irc_close(irc_con *const connection);

void irc_authenticate(irc_con *const connection, const char *nickname, const char *username, const char *realname, const char *pass);

int irc_read(irc_con *connection);
int irc_write(irc_con *connection);

int irc_listen(irc_con *connections, unsigned count);
