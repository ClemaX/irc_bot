#pragma once

typedef unsigned char irc_reply;

typedef struct irc_con_s irc_con;

typedef void (*irc_handler)(irc_con *connection, const char *message);
typedef void (*irc_msg_handler)(irc_con *connection, const char *sender, const char *message);

typedef struct irc_handler_lst_s
{
	irc_handler handler;
	struct irc_handler_lst_s *next;
} irc_handler_lst;

irc_handler_lst *irc_handler_new(irc_handler handler);
void irc_handler_del(irc_handler_lst **elem);
void irc_handler_clr(irc_handler_lst **lst);

int irc_handler_add(irc_handler_lst **lst, irc_handler handler);
void irc_handler_add_back(irc_handler_lst **lst, irc_handler_lst *elem);
void irc_handler_add_front(irc_handler_lst **lst, irc_handler_lst *elem);

int irc_on(irc_con *connection, irc_reply code, irc_handler handler);
int irc_on_message(irc_con *connection, irc_msg_handler handler);
void irc_off(irc_con *connection, irc_reply code, irc_handler handler);

void irc_handlers_init(irc_handler_lst *handlers[IRC_RPL_MAX]);
void irc_handlers_clr(irc_handler_lst *handlers[IRC_RPL_MAX]);
