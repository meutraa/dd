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

#ifndef DD_SENDING_H_
#define DD_SENDING_H_

void start_sending_thread(dc_context_t *context);

void send_voice(dc_context_t *context, const char *address, const char* file);
void send_audio(dc_context_t *context, const char *address, const char* file);
void send_image(dc_context_t *context, const char *address, const char* file);
void send_video(dc_context_t *context, const char *address, const char* file);
void send_file(dc_context_t *context, const char *address, const char* file);
void send_text(dc_context_t *context, char *address, char *message);

#endif
