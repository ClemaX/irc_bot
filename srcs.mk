# Sources
SRCDIR = src

SRCS = $(addprefix $(SRCDIR)/,\
	client.c\
	$(addprefix irc/,\
		address.c\
		buffer.c\
		connection.c\
		handlers.c\
	)\
)
