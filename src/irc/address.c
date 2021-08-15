#include <stdio.h>

#include <irc/address.h>

int irc_getaddr(struct addrinfo **addr, const char *hostname, const char *service)
{
	static const struct addrinfo hints =
		{
			.ai_family = AF_INET,
			.ai_socktype = SOCK_STREAM,
			.ai_protocol = IPPROTO_IP,
		};
	const int err = getaddrinfo(hostname, service, &hints, addr);

	if (err != 0)
	{
		printf("%s: Host not found!\n", hostname);
		if (err == EAI_SYSTEM)
			perror("getaddrinfo");
	}

	return err;
}
