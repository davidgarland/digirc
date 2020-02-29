/*
** digirc.c | Digi's IRC Bot | Main Proram
** https://github.com/davidgarland/digirc
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include "digirc.h"

void irc_cmd(TxtBuf *res, TxtBuf *cmd) {
  FILE *fp = popen(cmd->raw, "r");
  printf("[CMDS]: %s\n", cmd->raw);
  while (fgets(res->raw, 512, fp) != NULL);
  res->len = strlen(res->raw);
  printf("[RSLT]: %s", res->raw);
  pclose(fp);
}

void irc_loop(int conn, bool first, TxtBuf *buf) {
  TxtBuf nick = {0};
  TxtBuf msg = {0};
  TxtBuf args = {0};
  TxtBuf args_esc = {0};
  TxtBuf cmd = {0};
  TxtBuf res = {0};
  TxtBuf out = {0};
  enum server sv;

  txtbuf_alloc(&nick, 1);
  txtbuf_alloc(&msg, 1);
  txtbuf_alloc(&args, 1);
  txtbuf_alloc(&args_esc, 1);
  txtbuf_alloc(&cmd, 1);
  txtbuf_alloc(&res, 513);
  txtbuf_alloc(&out, 1);

  while (true) {
    irc_line(conn, buf);
    if (!strncmp(buf->raw, "PING", 4)) {
      buf->raw[1] = 'O';
      irc_send(conn, buf->raw);
      if (first)
        break;
    } else if (!first) {
      sv = irc_info(buf, &nick, &msg);
      printf("[RECV] %s | %s: %s\n", sv_name[sv], nick.raw, msg.raw);
      if (msg.len && msg.raw[0] == '%') {
        if (!strncmp(msg.raw, "\%reload", 7) && !strncmp(nick.raw, "Digi", 4)) {
          system("idris --O2 src/backend.idr -o backend");
        } else if (!strncmp(msg.raw, "\%eval", 5)) {

        } else if (!strncmp(msg.raw, "\%type", 5)) {

        } else {
          txtbuf_fmt(&args, "%s | %s: %s", sv_name[sv], nick.raw, msg.raw);
          shell_esc(&args_esc, &args);
          txtbuf_fmt(&cmd, "./backend %s", args_esc.raw);
          irc_cmd(&res, &cmd);
          if (!strncmp(res.raw, "OK", 2))
            continue;
          txtbuf_fmt(&out, "%s %s", nick.raw, res.raw);
          irc_send(conn, "PRIVMSG #openredstone :%s\r\n", out.raw);
        }
      }
    } else {
      printf("[INIT] %s", buf->raw);
    }
  }
}

int main() {
  TxtBuf buf = {0};
  txtbuf_alloc(&buf, 513);

  // Connect to the IRC server.
  struct addrinfo hints = {
    .ai_family = AF_INET,
    .ai_socktype = SOCK_STREAM
  };
  struct addrinfo *res;
  if (getaddrinfo("irc.esper.net", "6667", &hints, &res))
    exit(EXIT_FAILURE);
  int conn = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  connect(conn, res->ai_addr, res->ai_addrlen);
  irc_send(conn, "USER digirc 0 0 :digirc\r\n");
  irc_send(conn, "NICK digirc\r\n");

  // Respond to the initial ping.
  irc_loop(conn, true, &buf);

  // Join the room.
  while (true) {
    irc_line(conn, &buf);
    printf("[JOIN] %s", buf.raw);
    if (!strncmp(buf.raw, ":digirc", 7)) {
      irc_send(conn, "JOIN #openredstone\r\n");
      break;
    }
  }

  // Run the rest of the main loop.
  irc_loop(conn, false, &buf);

  return EXIT_SUCCESS;
}
