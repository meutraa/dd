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

// #include <stdio.h>

// #include <stdlib.h>

#include <deltachat.h>

#include "logging.h"
#include "output.h"
#include "input.h"
#include "network.h"

static void on_message_status_changed(dc_context_t *context, char *status, int chat_id,
                               int msg_id) {
  info("Message %s (chat: %d, message: %d)\n", status, chat_id, msg_id);
}

static void on_configured(dc_context_t *context) {
  print_all_messages(context);

  start_input_thread(context, dc_get_config(context, "accountdir", "/tmp/dd"));
}

uintptr_t on_event(dc_context_t *context, int event, uintptr_t data1,
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
