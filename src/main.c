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

#define _GNU_SOURCE

#include <deltachat.h>

#include "network.h"
#include "logging.h"
#include "sending.h"
#include "receiving.h"
#include "output.h"

#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <string.h>

static pthread_t listen_thread;

void *listen_to_pipe(void *context) {
  char *fifo = "/home/paul/.dd/in";

  int err = mkfifo(fifo, 0660);
  if (err == -1 && errno != EEXIST) { // !File exists
    fprintf(stderr, "Unable to create fifo: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  while (1) {
    FILE *fd = fopen(fifo, "r");
    if (fd == NULL) {
      fatal("Unable to open fifo for reading input");
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
        send_message(context, argv[0], argv[2]);
      } else if (0 == strcmp(argv[1], "file")) {
        // TODO
      }
    }
    fclose(fd);
  }
}

void on_message_status_changed(dc_context_t *context, char *status, int chat_id,
                               int msg_id) {
  info("Message %s (chat: %d, message: %d)\n", status, chat_id, msg_id);
}

void on_configured(dc_context_t *context) {
  print_all_messages(context);

  pthread_create(&listen_thread, NULL, listen_to_pipe, context);
}

uintptr_t event_callback(dc_context_t *context, int event, uintptr_t data1,
                         uintptr_t data2) {
  switch (event) {
  case 100:
    info("I: %s\n", (const char *)data2);
    break;
  case 300:
    info("W: %s\n", (const char *)data2);
    break;
  case 400:
    info("E: %d: %s\n", (int)data1, (const char *)data2);
    break;
  case 2005:
    print_message(context, (int)data1, (int)data2);
    break;
  case 2010: // Received sent message
    print_message(context, (int)data1, (int)data2);
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
  case 2100: {
    char *res = http_get((const char *)data1);
    if (NULL != res) {
      return (uintptr_t)res;
    }

    break;
  }
  case 2041:
    switch ((int)data1) {
    case 1000:
      info("I: %s\n", "Configured succesfully");
      on_configured(context);
      break;
    case 0:
      fatal("E: Unable to configure");
      break;
    }
    break;
  default:
    info("I: %s: %d\n", "Unhandled", event);
  }
  return 0;
}

int main(int argc, char **argv) {
  dc_context_t *context = dc_context_new(event_callback, NULL, NULL);

  // Parse command line options
  int c;
  while ((c = getopt(argc, argv, ":v")) != -1) {
    switch (c) {
    case 'v':
      logging = 1;
      break;
    }
  }

  // Get environment variables.
  char *email = getenv("DD_EMAIL");
  char *password = getenv("DD_PASSWORD");
  char *xdg_data_home = getenv("XDG_DATA_HOME");
  char *home = getenv("HOME");

  if (NULL == email) {
    fatal("DD_EMAIL not set");
  }
  if (NULL == password) {
    fatal("DD_PASSWORD not set");
  }
  if (NULL == home) {
    fatal("HOME not set");
  }

  // Create path strings
  char *datadir, *db, *accountdir;

  int res =
      asprintf(&datadir, NULL != xdg_data_home ? "%s/dd" : "%s/.local/share/dd",
               NULL != xdg_data_home ? xdg_data_home : home);
  if (-1 == res) {
    fatal("Unable to allocate memory");
  }

  res = asprintf(&accountdir, "%s/%s", datadir, email);
  if (-1 == res) {
    fatal("Unable to allocate memory");
  }

  res = asprintf(&db, "%s/db", accountdir);
  if (-1 == res) {
    fatal("Unable to allocate memory");
  }

  // Create necessary directories
  res = mkdir(datadir, S_IRWXU | S_IRWXG);
  if (-1 == res && errno != EEXIST) {
    fprintf(stderr, "Unable to create data directory: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  free(datadir);

  res = mkdir(accountdir, S_IRWXU | S_IRWXG);
  if (-1 == res && errno != EEXIST) {
    fprintf(stderr, "Unable to create account directory: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
  free(accountdir);

  dc_open(context, db, NULL);
  free(db);

  dc_set_config(context, "addr", email);
  dc_set_config(context, "mail_pw", password);
  dc_configure(context);

  start_receiving_thread(context);
  start_sending_thread(context);

  while (1) {
    sleep(1000);
  }
}
