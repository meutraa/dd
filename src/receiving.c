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
// #include <stdarg.h>

// #include <stdlib.h>
#include <deltachat.h>
#include <pthread.h>

static pthread_t imap_thread;

void *process_imap_tasks(void *context) {
  while (1) {
    dc_perform_imap_jobs(context);
    dc_perform_imap_fetch(context);
    dc_perform_imap_idle(context);
  }
}

void start_receiving_thread(dc_context_t* context) {
  pthread_create(&imap_thread, NULL, process_imap_tasks, context);
}
