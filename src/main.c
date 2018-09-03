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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "event_proxy.h"
#include "logging.h"
#include "config.h"
#include "output.h"
#include "receiving.h"
#include "sending.h"

#include <yaml.h>

#include "flag.h"

#define VERSION "v0.3.0"

int main(int argc, const char **argv) {
  char *email, *password, *xdg_data_home, *home, *datadir, *config_path;
  int res;

  // Create the data and config directory strings
  home = getenv("HOME");
  xdg_data_home = getenv("XDG_DATA_HOME");
  if (NULL == xdg_data_home) {
    res = asprintf(&datadir, "%s/%s", home, ".local/share/dd");
  } else {
    res = asprintf(&datadir, "%s/%s", xdg_data_home, "dd");
  }
  assert(-1 != res);

  res = asprintf(&config_path, "%s/.config/dd/dd.yaml", home);
  assert(-1 != res);

  // Parse config file first
  FILE *input = fopen(config_path, "rb");
  free(config_path);

  if (NULL != input) {
    int done = 0;
    yaml_parser_t parser;
    yaml_token_t token;
    char **setting;
    if (!yaml_parser_initialize(&parser)) {
      fatal("Unable to initialize yaml parser");
    }

    yaml_parser_set_input_file(&parser, input);
    while (!done) {
      if (!yaml_parser_scan(&parser, &token)) {
        break;
      }

      switch (token.type) {
      case YAML_STREAM_END_TOKEN:
        done = 1;
        break;
      case YAML_SCALAR_TOKEN:
        if (0 == strcmp(token.data.scalar.value, "email")) {
          setting = &email;
        } else if (0 == strcmp(token.data.scalar.value, "password")) {
          setting = &password;
        } else if (NULL != setting) {
          (*setting) = strdup(token.data.scalar.value);
          setting = NULL;
        } else {
          setting = NULL;
        }
        break;
      }
      if (token.type != YAML_STREAM_END_TOKEN) {
        yaml_token_delete(&token);
      }
    }

    yaml_parser_delete(&parser);
    fclose(input);
  }

  // Get environment variables.
  const char *env_email = getenv("DD_EMAIL");
  if (NULL != env_email) {
    email = env_email;
  }
  const char *env_password = getenv("DD_PASSWORD");
  if (NULL != env_password) {
    password = env_password;
  }

  // Command line arguments highest priority.
  flag_string(&email, "email", "Email address [DD_EMAIL]");
  flag_string(&password, "password", "Email password [DD_PASSWORD]");
  flag_string(&datadir, "datadir", "Data directory");
  flag_int(&history_count, "count", "Lines of history to print for each contact");
  flag_bool(&logging, "verbose", "Print debugging information");
  flag_parse(argc, argv, VERSION);

  if (NULL == email || NULL == password) {
    exit(EXIT_FAILURE);
  }

  // Check if data directory is writable
  res = access(datadir, W_OK);
  if (0 != res) {
    fatal("Unable to write to data directory");
  }

  // Create path strings
  char *db, *accountdir, *keydir;

  res = asprintf(&accountdir, "%s/%s", datadir, email);
  assert(-1 != res);

  res = asprintf(&db, "%s/db", accountdir);
  assert(-1 != res);

  res = asprintf(&keydir, "%s/%s", accountdir, "keys");
  assert(-1 != res);

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

  dc_context_t *context = dc_context_new(on_event, NULL, NULL);
  dc_open(context, db, NULL);
  free(db);

  dc_set_config(context, "addr", email);
  dc_set_config(context, "mail_pw", password);
  dc_set_config(context, "accountdir", accountdir);
  free(accountdir);

  dc_configure(context);

  info("Importing private keys from %s", keydir);
  dc_imex(context, DC_IMEX_IMPORT_SELF_KEYS, keydir, NULL);

  start_receiving_thread(context);
  start_sending_thread(context);

  while (1) {
    sleep(1000);
  }
}
