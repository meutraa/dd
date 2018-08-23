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

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct Data {
  char *data;
  size_t size;
};

static size_t curl_wc(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct Data *mem = (struct Data *)userp;

  mem->data = realloc(mem->data, mem->size + realsize + 1);
  if (mem->data == NULL) {
    printf("Cannot allocate memory for curl request");
    return 0;
  }

  memcpy(&(mem->data[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->data[mem->size] = 0;

  return realsize;
}

// Remember to free this return value. (Delta chat does it)
char *http_get(const char *address) {
  // Asking for a HTTP Get
  CURLcode ret;
  CURL *curl;
  struct Data chunk;

  chunk.data = malloc(1);
  chunk.size = 0;

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, address);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_wc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
  ret = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  /* check for errors */
  if (ret != CURLE_OK) {
    fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(ret));
    return NULL;
  }

  return chunk.data;
}
