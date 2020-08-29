/*
** irc.c | Digi's IRC Bot | IRC code.
** https://github.com/davidgarland/digirc
*/

#include <string.h>
#include <ctype.h>
#include "digirc.h"

char *sv_name[SV_LENGTH] = {
  [SV_NONE] = "None",
  [SV_IRC] = "Irc",
  [SV_DISCORD] = "Discord",
  [SV_NETWORK] = "Network"
};

void irc_send(int conn, const char *const fmt, ...) {
  static char buf[512];
  va_list ap;
  va_start(ap, fmt);
  register const uint16_t len = vsnprintf(buf, 512, fmt, ap);
  va_end(ap);
  printf("[SEND] %s", buf);
  write(conn, buf, len);
}

char irc_char(int conn) {
  char c;
  uint16_t bytes = read(conn, &c, 1);
  return c;

  /*
  static char buf[512] = {0};
  static uint16_t bytes = 0;
  static uint16_t bytes_left = 0;

  if (!bytes_left) {
    bytes = 0;
    while (!bytes)
      bytes = read(conn, buf, 512);
    bytes_left = bytes;
  }

  return buf[bytes - bytes_left--];
  */
}

void irc_line(int conn, TxtBuf *buf) {
  txtbuf_clear(buf);

  bool last_r = false;

  while (true) {
    char c = irc_char(conn);
    txtbuf_push(buf, c);
    if (last_r && (c == '\n'))
      return;
    last_r = c == '\r';
  }
}

void irc_nick_data(TxtBuf *buf, TxtBuf *nick) {
  txtbuf_clear(nick);

  for (size_t i = 1, j = 0; (i < (buf->len - 2)) && (buf->data[i] != '!'); i++, j++)
    txtbuf_push(nick, buf->data[i]);
}

void irc_msg_data(TxtBuf *buf, TxtBuf *msg) {
  txtbuf_clear(msg);

  size_t i, j;
  
  for (i = 0, j = 0; (i < (buf->len - 2)) && (j < 3); i++)
    if (buf->data[i] == ' ')
      j++;
  i++;
  
  for (j = 0; i < (buf->len - 2); i++, j++)
    txtbuf_push(msg, buf->data[i]);
}

enum server irc_info(TxtBuf *buf, TxtBuf *nick, TxtBuf *msg) {
  irc_nick_data(buf, nick);
  irc_msg_data(buf, msg);

  enum server sv = SV_IRC;
  if (!strncmp(nick->data, "ORENetwork", 10))
    sv = SV_NETWORK;
  else if (!strncmp(nick->data, "OREDiscord", 10))
    sv = SV_DISCORD;

  if (sv != SV_IRC) {
    uint16_t i, j, k;
    for (i = 0; (i < msg->len) && (msg->data[i] != ':'); i++);
    for (j = 0; j < (i - 4); j++)
      txtbuf_set(nick, j, msg->data[j + 3]);
    txtbuf_set(nick, j, '\0');
    for (j = i + 2, k = 0; j < msg->len; j++, k++)
      txtbuf_set(msg, k, msg->data[j]);
    txtbuf_set(msg, k, '\0');
  }

  return sv;
}

void shell_esc(TxtBuf *dst, TxtBuf *src) {
  txtbuf_clear(dst);
  txtbuf_push(dst, '\'');
  for (size_t i = 0; i < src->len; i++) {
    char c = src->data[i];
    printf("%c", c);
    if (c == '\'') {
      txtbuf_cat_cstr(dst, "\'\"\'\"\'");
    } else {
      txtbuf_push(dst, c);
    }
  }
  txtbuf_push(dst, '\'');
}

