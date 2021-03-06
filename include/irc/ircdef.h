#pragma once

#define IRC_HANDLER_MAX 512U
#define IRC_MESSAGE_MAXLEN 512U
#define IRC_MESSAGE_DELIM ' '
#define IRC_MESSAGE_PREFIX_PREFIX ':'
#define IRC_MESSAGE_LAST_ARG_PREFIX ':'
#define IRC_MESSAGE_SUFFIX "\r\n"

/// Maximal simultaneous connections
#define IRC_CONNECTION_MAX 10U
/// Connection poll timeout
#define IRC_CONNECTION_TIMEOUT (60 * 1000)
/// Read chunk size
#define IRC_READ_CHUNK_SIZE 32U
