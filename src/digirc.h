/*
** irc.h | Digi's IRC Bot | Main Header
** https://github.com/davidgarland/digirc
*/

#ifndef DIGIRC_H
#define DIGIRC_H

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>

#define CIRCA_LOGGING
#include <circa_txtbuf.h>

enum server {
  SV_NONE,
  SV_IRC,
  SV_DISCORD,
  SV_NETWORK,
  SV_LENGTH
};

extern char *sv_name[SV_LENGTH];

void irc_send(int conn, const char *const fmt, ...);
char irc_char(int conn);
void irc_line(int conn, TxtBuf *out);

void irc_nick_raw(TxtBuf *buf, TxtBuf *nick);
void irc_msg_raw(TxtBuf *buf, TxtBuf *msg);

enum server irc_info(TxtBuf *buf, TxtBuf *nick, TxtBuf *msg);

void shell_esc(TxtBuf *dst, TxtBuf *src);

#endif // DIGIRC_H
