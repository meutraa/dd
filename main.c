/*
 *  This file is part of dd.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dd.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Copyright 2018 Paul Meredith
 */

#include <deltachat.h>

#include "network.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <string.h>

static pthread_t imap_thread, smtp_thread, listen_thread;

void *process_imap_tasks(void *context) {
  while (1) {
    dc_perform_imap_jobs(context);
    dc_perform_imap_fetch(context);
    dc_perform_imap_idle(context);
  }
}

void *process_smtp_tasks(void *context) {
  while (1) {
    dc_perform_smtp_jobs(context);
    dc_perform_smtp_idle(context);
  }
}

void on_fatal_error(dc_context_t *context, char *message) {
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

void send_message(dc_context_t *context, char *address, char *message) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_text_msg(context, chat_id, message);
}

void *listen_to_pipe(void *context) {
  char *fifo = "/home/paul/.dd/in";

  int err = mkfifo(fifo, 0660);
  if (err == -1) {
    fprintf(stderr, "Unable to create fifo: %s\n", strerror(errno));
    // exit(EXIT_FAILURE);
  }

  while (1) {
    FILE *fd = fopen(fifo, "r");
    if (fd == NULL) {
      on_fatal_error(NULL, "Unable to open fifo for reading input");
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, fd);

    if (read != -1) {

      char **ap, *argv[3];

      for (ap = argv; (*ap = strsep(&line, "|")) != NULL;) {
        if (**ap != '\0') {
          if (++ap >= &argv[3]) {
            break;
          }
        }
      }

      if (0 == strcmp(argv[1], "message")) {
        fprintf(stderr, "Received instruction to send a message to %s\n",
                argv[0]);
        send_message(context, argv[0], argv[2]);
      } else if (0 == strcmp(argv[1], "file")) {
      }
    }
    fclose(fd);
  }
}

void on_message_status_changed(dc_context_t *context, char *status, int chat_id,
                               int msg_id) {
  fprintf(stderr, "Message %s (chat: %d, message: %d)\n", status, chat_id,
          msg_id);
}

void print_message(dc_context_t *context, int chat_id, int msg_id) {
  dc_chat_t *chat = dc_get_chat(context, chat_id);
  char *name = dc_chat_get_name(chat);

  dc_msg_t *msg = dc_get_msg(context, msg_id);
  char *text = dc_msg_get_text(msg);

  uint32_t sender_id = dc_msg_get_from_id(msg);
  time_t received_at = dc_msg_get_timestamp(msg);

  char buff[20];
  strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&received_at));

  if (1 == sender_id) {
    printf("%s\t%s: %s\n", buff, "toka@lost.host", text);
  } else {
    printf("%s\t%s: %s\n", buff, name, text);
  }

  free(name);
  free(text);

  dc_chat_unref(chat);
  dc_msg_unref(msg);
}

// This is just for testing right now.
void print_all_messages(dc_context_t *context) {
  dc_chatlist_t *chatlist = dc_get_chatlist(context, 0, NULL, 0);

  for (int j = 0; j < dc_chatlist_get_cnt(chatlist); j++) {

    uint32_t chat_id = dc_chatlist_get_chat_id(chatlist, j);
    dc_array_t *msglist = dc_get_chat_msgs(context, chat_id, 0, 0);

    for (int i = 0; i < dc_array_get_cnt(msglist); i++) {
      uint32_t msg_id = dc_array_get_id(msglist, i);
      print_message(context, chat_id, msg_id);
    }

    dc_array_unref(msglist);
  }

  dc_chatlist_unref(chatlist);
}

void on_configured(dc_context_t *context) {
  print_all_messages(context);

  pthread_create(&listen_thread, NULL, listen_to_pipe, context);
}

uintptr_t event_callback(dc_context_t *context, int event, uintptr_t data1,
                         uintptr_t data2) {
  switch (event) {
  case 100:
    fprintf(stderr, "Info: %s\n", (const char *)data2);
    break;
  case 300:
    fprintf(stderr, "Warning: %s\n", (const char *)data2);
    break;
  case 400:
    fprintf(stderr, "Error (%d): %s\n", (int)data1, (const char *)data2);
    break;
  case 2005:
    print_message(context, (int)data1, (int)data2);
    break;
  case 2010:
    on_message_status_changed(context, "Delivered", (int)data1, (int)data2);
    break;
  case 2012:
    on_message_status_changed(context, "Failed", (int)data1, (int)data2);
    break;
  case 2015:
    on_message_status_changed(context, "Read", (int)data1, (int)data2);
    break;
  case 2091:
    // Asking for a localized string, 0 for default
    break;
  case 2100:
    fprintf(stderr, "HTTP Get: %s\n", (const char *)data1);

    char *res = http_get((const char *)data1);
    if (NULL != res) {
      return (uintptr_t)res;
    }

    break;
  case 2041:
    fprintf(stderr, "Configuring: %d\n", (int)data1);
    switch ((int)data1) {
    case 1000:
      on_configured(context);
      break;
    case 0:
      on_fatal_error(context, "Unable to configure");
      break;
    }
    break;
  default:
    fprintf(stdout, "Unhandled: %d\n", event);
  }
  return 0;
}

int main() {
  dc_context_t *context = dc_context_new(event_callback, NULL, NULL);

  char* email = getenv("DD_EMAIL")
  char* password = getenv("DD_PASSWORD")

  if (NULL == email || NULL == password) {
    fprintf(stderr, "%s\n", "DD_EMAIL or DD_PASSWORD not set");
    exit(EXIT_FAILURE)
  }

  dc_open(context, "data.db", NULL);

  dc_set_config(context, "addr", email);
  dc_set_config(context, "mail_pw", password);
  dc_configure(context);

  pthread_create(&imap_thread, NULL, process_imap_tasks, context);
  pthread_create(&smtp_thread, NULL, process_smtp_tasks, context);

  while (1) {
    sleep(1000);
  }
}
