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
#include <pthread.h>
#include <stdio.h>

#include "strings.h"

static pthread_t smtp_thread;

static void *process_smtp_tasks(void *context) {
  while (1) {
    dc_perform_smtp_jobs(context);
    dc_perform_smtp_idle(context);
  }
  return NULL;
}

void start_sending_thread(dc_context_t *context) {
  pthread_create(&smtp_thread, NULL, process_smtp_tasks, context);
}

void send_voice(dc_context_t *context, const char *address, const char *file) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_voice_msg(context, chat_id, file, "Audio/Ogg", 0);
}

void send_audio(dc_context_t *context, const char *address, const char *file) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_audio_msg(context, chat_id, file, NULL, 0, NULL, NULL);
}

void send_image(dc_context_t *context, const char *address, const char *file) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_image_msg(context, chat_id, file, NULL, 0, 0);
}

void send_video(dc_context_t *context, const char *address, const char *file) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_video_msg(context, chat_id, file, NULL, 0, 0, 0);
}

void send_file(dc_context_t *context, const char *address, const char *file) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  dc_send_file_msg(context, chat_id, file, NULL);
}

void send_text(dc_context_t *context, const char *address, char *message) {
  uint32_t contact_id = dc_create_contact(context, NULL, address);
  uint32_t chat_id = dc_create_chat_by_contact_id(context, contact_id);

  trim(message);
  dc_send_text_msg(context, chat_id, message);
}
