#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <zlib.h>

#include <irc/connection.h>

#include <base64.h>

#include <logger.h>

static char *rotate(const char *data, char amount) {
  size_t length = strlen(data);
  char *rotated = malloc(sizeof(*rotated) * (length + 1));

  if (rotated != NULL) {
    while (*data != '\0') {
      if (isupper(*data))
        *rotated++ = 'A' + (*data++ - 'A' + amount) % ('Z' - 'A' + 1);
      else if (islower(*data))
        *rotated++ = 'a' + (*data++ - 'a' + amount) % ('z' - 'a' + 1);
      else
        *rotated++ = *data++;
    }
    *rotated = *data;
    rotated -= length;
  }
  return rotated;
}

static void on_welcome(irc_con *const connection,
                       const irc_msg *const message) {
  printf("Received welcome '%s' on fd %d!\n", message->id, connection->fd);
  irc_buff_append(&connection->write, "PRIVMSG Candy :!ep3" IRC_MESSAGE_SUFFIX);
}

static void on_message(irc_con *const connection,
                       const irc_msg *const message) {
  char *rotated;

  if (message->arguments != NULL && message->arguments[0] != NULL &&
      message->arguments[1] != NULL &&
      strcmp("Candy!Candy@root-me.org", message->prefix) == 0) {
    printf("Received message '%s' on fd %d!\n", message->arguments[1],
           connection->fd);

    rotated = rotate(message->arguments[1], 13);
    printf("rotated: '%s'\n", rotated);
    if (rotated != NULL) {
      irc_buff_append(&connection->write, "PRIVMSG Candy !ep3 -rep :");
      irc_buff_append(&connection->write, rotated);
      irc_buff_append(&connection->write, IRC_MESSAGE_SUFFIX);
      free(rotated);
    }
  }
}

int main(int ac, const char **av) {
  (void)ac;
  (void)av;

  SSL_CTX *ssl_context = NULL;
  irc_con *connection;

  // ssl_context = irc_create_ssl_context();
  // if (ssl_context != NULL)
  // {
  connection = irc_con_new(ssl_context);
  if (connection != NULL) {
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
