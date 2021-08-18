# Sources
SRCDIR = src

SRCS = $(addprefix $(SRCDIR)/,\
	$(addprefix irc/,\
		address.c\
		buffer.c\
		connection.c\
		handlers.c\
		message.c\
	)\
)
