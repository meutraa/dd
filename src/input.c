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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "logging.h"
#include "sending.h"

static pthread_t listen_thread;

struct thread_args {
  dc_context_t *context;
  const char *accountdir;
};

typedef void (*sendFunc)(struct _dc_context *, const char *, const char *);

struct ext {
  char *ext;
  sendFunc function;
};

struct ext exts[] = {
    {"jpg", &send_image},  {"jpeg", &send_image}, {"JPG", &send_image},
    {"png", &send_image},  {"mp3", &send_audio},  {"ogg", &send_voice},
    {"opus", &send_voice}, {"wav", &send_voice},  {"mp4", &send_video},
};

static sendFunc get_ext_function(const char *extension) {
  for (unsigned long i = 0; i < sizeof(exts) / sizeof(exts[0]); i++) {
    if (0 == strcmp(exts[i].ext, extension)) {
      return exts[i].function;
    }
  }
  return NULL;
}

static void *start_listen_thread(void *arguments) {
  struct thread_args *args = (struct thread_args *)arguments;

  char *fifo;
  int res = asprintf(&fifo, "%s/in", args->accountdir);
  if (-1 == res) {
    fatal("Unable to allocate memory");
  }

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
        send_text(args->context, argv[0], argv[2]);
      } else {
        char *ext = argv[1];
        char *path = argv[2];

        if (NULL != ext && NULL != path && strlen(path) > 0 &&
            strlen(ext) > 0) {
          sendFunc func = get_ext_function(ext);
          if (NULL == func) {
            send_file(args->context, argv[0], path);
          } else {
            func(args->context, argv[0], path);
          }
        }
      }
    }
    fclose(fd);
  }
  free(arguments);
}

void start_input_thread(dc_context_t *context, const char *accountdir) {
  struct thread_args *args = malloc(sizeof(struct thread_args));
  args->context = context;
  args->accountdir = accountdir;
  pthread_create(&listen_thread, NULL, start_listen_thread, args);
}
