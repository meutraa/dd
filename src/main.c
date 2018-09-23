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

#include <assert.h>
#include <deltachat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "event_proxy.h"
#include "logging.h"
#include "output.h"
#include "receiving.h"
#include "sending.h"

#include "flag.h"
#include "ini.h"

#define VERSION "v0.3.0"

typedef struct {
  char *value;
  char *opt_name;
  char *env_name;
  char *arg_name;
  char *ini_section;
  char *ini_name;
  char *description;
} Option;

char *getValue(Option options[], int len, char *opt_name) {
  for (int i = 0; i < len; i++) {
    if(0 == strcmp(opt_name, options[i].opt_name)) {
      if (0 == strcmp("", options[i].value)) {
        return NULL;
      }
      return options[i].value;
    }
  }
  return NULL;
}

// All off these options will be freed when ini_free is called.
void from_ini(Option *option, ini_t *config) {
  if (NULL == config) {
    return;
  }
  const char *opt = ini_get(config, option->ini_section, option->ini_name);
  if (NULL != opt && 0 != strcmp("", opt)) {
    option->value = opt;
  }
}

void from_env(Option *option) {
  const char *opt = getenv(option->env_name);
  if (NULL != opt && 0 != strcmp("", opt)) {
    option->value = opt;
  }
}

void from_arg(Option *option) {
  flag_string(&option->value, option->arg_name, option->description);
}

void set_option(dc_context_t *context, Option *option) {
  if (NULL == option->opt_name || 0 == strcmp("", option->opt_name)) {
    return;
  }
  if (NULL != option->value && 0 != strcmp("", option->value)) {
    info("Setting %s to %s", option->opt_name, option->value);
    dc_set_config(context, option->opt_name, option->value);
  }
}

int main(int argc, const char **argv) {
  char *datadir = "";
  char *config_path = "";
  int res;

  // Create the data and config directory strings
  char *env_home = getenv("HOME");
  char *env_xdg_data_home = getenv("XDG_DATA_HOME");
  if (NULL == env_xdg_data_home) {
    res = asprintf(&datadir, "%s/%s", env_home, ".local/share/dd");
  } else {
    res = asprintf(&datadir, "%s/%s", env_xdg_data_home, "dd");
  }
  assert(-1 != res);

  res = asprintf(&config_path, "%s/.config/dd/dd.ini", env_home);
  assert(-1 != res);

  Option options[] = {
      {"", "mail_server", "DD_IMAP_SERVER", "imap-server", "imap",
       "server", "Incoming mail server"},
      {"", "mail_user", "DD_IMAP_USER", "imap-user", "imap", "user",
       "Incoming username"},
      {"", "mail_port", "DD_IMAP_PORT", "imap-port", "imap", "port",
       "Incoming port"},
      {"", "mail_pw", "DD_IMAP_PASSWORD", "imap-password", "imap",
       "password", "Incoming password"},

      {"", "send_server", "DD_SMTP_SERVER", "smtp-server", "smtp",
       "server", "Outgoing mail server"},
      {"", "send_user", "DD_SMTP_USER", "smtp-user", "smtp", "user",
       "Outgoing username"},
      {"", "send_port", "DD_SMTP_PORT", "smtp-port", "smtp", "port",
       "Outgoing port"},
      {"", "send_pw", "DD_SMTP_PASSWORD", "smtp-password", "smtp",
       "password", "Outgoing password"},

      {"", "addr", "DD_EMAIL", "email", "account", "email", "Email address"},
      {"", "displayname", "DD_DISPLAY_NAME", "display-name",
       "account", "displayname", "Display name"},
      {"", "selfstatus", "DD_FOOTER", "footer", "account", "footer",
       "Email signature"},
  };

  int opt_len = sizeof(options)/sizeof(Option);

  // Parse config file first
  config = ini_load(config_path);
  for (int i = 0; i < opt_len; i++) {
    from_ini(&options[i], config);
    from_env(&options[i]);
    from_arg(&options[i]);
  }

  // flag_string(&datadir, "datadir", "Data directory");
  flag_int(&history_count, "count",
           "Lines of history to print for each contact");
  flag_bool(&alarm_disabled, "silent", "Silence the alarm bell");
  flag_bool(&logging, "verbose", "Print debugging information");
  flag_parse(argc, argv, VERSION);

  // Options do not change after this point!
  char *addr = getValue(options, opt_len, "addr");
  char *mail_pw = getValue(options, opt_len, "mail_pw");

  // Check required variables
  if (NULL == addr || NULL == mail_pw) {
    info("addr or mail_pw is %s", mail_pw);
    exit(EXIT_FAILURE);
  }

  // Check if data directory is writable
  res = access(datadir, W_OK);
  if (0 != res) {
    fatal("Unable to write to data directory");
  }

  // Create path strings
  char *db, *accountdir, *keydir;

  res = asprintf(&accountdir, "%s/%s", datadir, addr);
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

  int option_count = sizeof(options) / sizeof(Option);
  for (int i = 0; i < option_count; i++) {
    set_option(context, &options[i]);
  }
  dc_set_config(context, "accountdir", accountdir);
  free(accountdir);

  // I use this in the output thread.
  // free(config);

  dc_configure(context);

  //info("Importing private keys from %s", keydir);
  //dc_imex(context, DC_IMEX_IMPORT_SELF_KEYS, keydir, NULL);

  start_receiving_thread(context);
  start_sending_thread(context);

  while (1) {
    sleep(1000);
  }
}
