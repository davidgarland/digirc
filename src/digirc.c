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
  FILE *fp = popen(cmd->data, "r");
  printf("[CMDS]: %s\n", cmd->data);
  txtbuf_read(res, fp);
  printf("[RSLT]: %s", res->data);
  pclose(fp);
}

void mueval(TxtBuf *res, bool type, TxtBuf *cmd, TxtBuf *args) {
  txtbuf_fmt(cmd, "stack exec -- mueval --module Data.Complex --module Data.Void --module Data.List --module Data.Tree --module Data.Functor --module Control.Monad --module Control.Comonad --module Control.Lens --module Data.Monoid -t 20 %s -e %s +RTS -N2 -RTS", type ? "--inferred-type -T" : "", args->data);
  FILE *fp = popen(cmd->data, "r");
  if (!fp) {
    txtbuf_cpy_cstr(res, "Error");
    return;
  }
  int i = 0;
  while (txtbuf_readline(res, fp) != CE_EOF) {
    printf("%i: %s\n", i, res->data);
    if ((i == 0) && strstr(res->data, "error")) {
      txtbuf_cpy_cstr(res, "Error");
      break;
    }
    if ((i == 0) && !type)
      break;
    if ((i == 1) && type)
      break;
    i++;
  }
  if (pclose(fp)) {
    txtbuf_cpy_cstr(res, "Error");
    return;
  }
}

void irc_loop(int conn, bool first, TxtBuf *buf) {
  TxtBuf nick = txtbuf_init();
  TxtBuf msg = txtbuf_init();
  TxtBuf args = txtbuf_init();
  TxtBuf args_esc = txtbuf_init();
  TxtBuf cmd = txtbuf_init();
  TxtBuf res = txtbuf_init();
  TxtBuf out = txtbuf_init();
  enum server sv;

  txtbuf_alloc(&nick, 1);
  txtbuf_alloc(&msg, 1);
  txtbuf_alloc(&args, 1);
  txtbuf_alloc(&args_esc, 1);
  txtbuf_alloc(&cmd, 1);
  txtbuf_alloc(&res, 2049);
  txtbuf_alloc(&out, 1);

  while (true) {
    irc_line(conn, buf);
    if (!strncmp(buf->data, "PING", 4)) {
      buf->data[1] = 'O';
      irc_send(conn, buf->data);
      if (first)
        break;
    } else if (!first) {
      sv = irc_info(buf, &nick, &msg);
      printf("[RECV] %s | %s: %s\n", sv_name[sv], nick.data, msg.data);
      if (msg.len && msg.data[0] == '.') {
        if (!strncmp(msg.data, ".reload", 7) && !strncmp(nick.data, "Digi", 4)) {
          system("idris --O2 src/backend.idr -o backend");
        } else if (!strncmp(msg.data, ".eval", 5) && msg.len > 6) {
          txtbuf_cpy_cstr(&args, msg.data + 6);
          shell_esc(&args_esc, &args);
          mueval(&res, false, &cmd, &args_esc);
          txtbuf_fmt(&out, "%s => %s", nick.data, res.data);
          irc_send(conn, "PRIVMSG #openredstone :%s\r\n", out.data);
        } else if (!strncmp(msg.data, ".type", 5) && msg.len > 6) {
          txtbuf_cpy_cstr(&args, msg.data + 6);
          shell_esc(&args_esc, &args);
          mueval(&res, true, &cmd, &args_esc);
          txtbuf_fmt(&out, "%s => %s", nick.data, res.data);
          irc_send(conn, "PRIVMSG #openredstone :%s\r\n", out.data);
        } else {
          txtbuf_fmt(&args, "%s | %s: %s", sv_name[sv], nick.data, msg.data);
          printf("args: %s\n", args.data);
          shell_esc(&args_esc, &args);
          txtbuf_fmt(&cmd, "./backend %s", args_esc.data);
          irc_cmd(&res, &cmd);
          if (!strncmp(res.data, "OK", 2))
            continue;
          txtbuf_fmt(&out, "%s %s", nick.data, res.data);
          irc_send(conn, "PRIVMSG #openredstone :%s\r\n", out.data);
        }
      }
    } else {
      printf("[INIT] %s", buf->data);
    }
  }
}

int main() {
  TxtBuf buf = {0};
  txtbuf_alloc(&buf, 513);
  buf.data[512] = '\0';

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
    printf("[JOIN] %s", buf.data);
    if (!strncmp(buf.data, ":digirc", 7)) {
      irc_send(conn, "JOIN #openredstone\r\n");
      break;
    }
  }

  // Run the rest of the main loop.
  irc_loop(conn, false, &buf);

  return EXIT_SUCCESS;
}
