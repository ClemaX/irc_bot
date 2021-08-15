#pragma once

#include <netdb.h>

int irc_getaddr(struct addrinfo **addr, const char *hostname, const char *service);
