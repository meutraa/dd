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

#include "logging.h"
#include "sending.h"
#include "receiving.h"
#include "output.h"
#include "event_proxy.h"

#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <string.h>

int main(int argc, char **argv) {
  dc_context_t *context = dc_context_new(on_event, NULL, NULL);

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

  dc_open(context, db, NULL);
  free(db);

  dc_set_config(context, "addr", email);
  dc_set_config(context, "mail_pw", password);
  dc_set_config(context, "accountdir", accountdir);
  free(accountdir);

  dc_configure(context);

  start_receiving_thread(context);
  start_sending_thread(context);

  while (1) {
    sleep(1000);
  }
}
