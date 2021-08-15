#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <irc/connection.h>
#include <irc/handlers.h>

void irc_handler_add_back(irc_handler_lst **lst, irc_handler_lst *elem)
{
	irc_handler_lst *curr;

	if (*lst == NULL)
		*lst = elem;
	else
	{
		curr = *lst;
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = elem;
	}
}

void irc_handler_add_front(irc_handler_lst **lst, irc_handler_lst *elem)
{
	elem->next = *lst;
	*lst = elem;
}

int irc_handler_add(irc_handler_lst **lst, irc_handler handler)
{
	irc_handler_lst *curr = *lst;
	int ret;

	ret = 0;

	while (curr && curr->handler != handler)
		curr = curr->next;

	if (curr == NULL)
	{
		curr = irc_handler_new(handler);
		if (curr == NULL)
			ret = -2;
		*lst = curr;
	}
	else
		ret = -1;
	return ret;
}

irc_handler_lst *irc_handler_new(irc_handler handler)
{
	irc_handler_lst *const elem = malloc(sizeof(*elem));

	if (elem)
		*elem = (irc_handler_lst){handler, NULL};
	else
		perror("malloc");
	return elem;
}

void irc_handler_del(irc_handler_lst **elem)
{
	free(*elem);
	*elem = NULL;
}

int irc_on(irc_con *connection, irc_reply code, irc_handler handler)
{
	return irc_handler_add(&connection->handlers[code], handler);
}

int irc_on_message(irc_con *connection, irc_msg_handler handler)
{
	return irc_handler_add(&connection->msg_handlers, (irc_handler)handler);
}

void irc_off(irc_con *connection, irc_reply code, irc_handler handler)
{
	irc_handler_lst *curr = connection->handlers[code];
	irc_handler_lst *prev = NULL;

	while (curr != NULL && curr->handler != handler)
	{
		prev = curr;
		curr = curr->next;
	}

	if (curr != NULL)
	{
		if (prev != NULL)
			prev->next = curr->next;
		free(curr);
	}
}

void irc_handlers_init(irc_handler_lst *handlers[IRC_RPL_MAX])
{
	bzero(handlers, IRC_RPL_MAX);
}

void irc_handlers_clr(irc_handler_lst *handlers[IRC_RPL_MAX])
{
	for (unsigned i = 0; i < IRC_RPL_MAX; i++)
	{
		if (handlers[i] != NULL)
			irc_handler_clr(&handlers[i]);
	}
}

void irc_handler_clr(irc_handler_lst **lst)
{
	irc_handler_lst *next = *lst;
	irc_handler_lst *curr;

	*lst = NULL;
	while (next)
	{
		curr = next;
		next = curr->next;
		free(curr);
	}
}
