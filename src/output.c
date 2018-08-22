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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *last_room;

void print_message(dc_context_t *context, int chat_id, int msg_id) {
  dc_msg_t *msg = dc_get_msg(context, msg_id);
  uint32_t sender_id = dc_msg_get_from_id(msg);
  dc_chat_t *chat = dc_get_chat(context, chat_id);
  dc_contact_t *sender = dc_get_contact(context, sender_id);
  time_t received_at = dc_msg_get_timestamp(msg);

  char *name = dc_contact_get_display_name(sender);
  char *text = dc_msg_get_text(msg);

  // Same room logic.
  char *room = dc_chat_get_name(chat);
  int same = NULL != last_room && 0 == strcmp(room, last_room) ? 1 : 0;
  if (NULL != last_room) {
    free(last_room);
  }
  last_room = strdup(room);

  // Get the time the message was received.
  char buff[6];
  strftime(buff, 6, "%H:%M", localtime(&received_at));

  // TODO: check if text ever does not end with a new line.
  if (!same) {
    printf("%s\n", room);
  }

  char *filepath = dc_msg_get_file(msg);
  char *body = '\0' != filepath[0] ? filepath : text;

  printf("\t%s\t%s: %s\n", buff, name, body);
  fflush(stdout);

  free(filepath);
  free(room);
  free(name);
  free(text);

  dc_chat_unref(chat);
  dc_msg_unref(msg);
  dc_contact_unref(sender);
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
