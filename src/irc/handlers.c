#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <irc/connection.h>
#include <irc/handler.h>

#include <logger.h>

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

	debug("Adding handler %p...\n", handler);
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

	if (elem != NULL)
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

int irc_on(irc_con *connection, const char *command, irc_handler handler)
{
	map_pair *pair = hash_map_get(connection->handlers, command);
	int ret;

	if (pair == NULL)
		pair = hash_map_set(connection->handlers, command, NULL);
	if (pair != NULL)
		ret = irc_handler_add(((irc_handler_lst **)&pair->value), handler);
	else
		ret = -2;

	return ret;
}

void irc_off(irc_con *connection, const char *command, irc_handler handler)
{
	map_pair *const pair = hash_map_get(connection->handlers, command);
	irc_handler_lst *curr;
	irc_handler_lst *prev = NULL;

	if (pair != NULL)
	{
		curr = (irc_handler_lst *)pair->value;

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
}

void irc_handler_clr(irc_handler_lst *lst)
{
	irc_handler_lst *curr;

	while (lst)
	{
		curr = lst;
		lst = lst->next;
		free(curr);
	}
}

void irc_handler_dispatch(irc_con *const connection, const irc_msg *const message)
{
	const map_pair *const pair = hash_map_get(connection->handlers, message->id);
	const irc_handler_lst *handler_lst;

	if (pair != NULL)
	{
		handler_lst = (irc_handler_lst *)pair->value;
		debug("Checking pair %p...\n", pair);
		while (handler_lst != NULL)
		{
			debug("Handler for id: '%s' -> '%p'!\n", message->id, handler_lst->handler);
			handler_lst->handler(connection, message);
			handler_lst = handler_lst->next;
		}
	}
}
