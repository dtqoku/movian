/*
 *  Showtime Mediacenter
 *  Copyright (C) 2007-2014 Lonelycoder AB
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This program is also available under a commercial proprietary license.
 *  For more information, contact andreas@lonelycoder.com
 */

#include <unistd.h>
#include <assert.h>

#include "ecmascript.h"

#if ENABLE_WEBPOPUP
#include "ui/webpopup.h"
#endif

#include "misc/str.h"

/**
 *
 */
static int
es_webpopup(duk_context *ctx)
{

  duk_push_object(ctx);

#if ENABLE_WEBPOPUP
  const char *url   = duk_safe_to_string(ctx, 0);
  const char *title = duk_safe_to_string(ctx, 1);
  const char *trap  = duk_safe_to_string(ctx, 2);

  webpopup_result_t *wr = webpopup_create(url, title, trap);

  const char *t;
  switch(wr->wr_resultcode) {
  case WEBPOPUP_TRAPPED_URL:
    t = "trapped";
    break;
  case WEBPOPUP_CLOSED_BY_USER:
    t = "userclose";
    break;
  case WEBPOPUP_LOAD_ERROR:
    t = "neterror";
    break;
  default:
    t = "error";
    break;
  }



  duk_push_string(ctx, t);
  duk_put_prop_string(ctx, -2, "result");

  if(wr->wr_trapped.url != NULL) {
    duk_push_string(ctx, wr->wr_trapped.url);
    duk_put_prop_string(ctx, -2, "trappedUrl");
  }

  duk_push_object(ctx);

  http_header_t *hh;
  LIST_FOREACH(hh, &wr->wr_trapped.qargs, hh_link) {
    duk_push_string(ctx, hh->hh_value);
    duk_put_prop_string(ctx, -2, hh->hh_key);
  }
  duk_put_prop_string(ctx, -2, "args");

  webpopup_result_free(wr);
#else

  duk_push_string(ctx, "unsupported");
  duk_put_prop_string(ctx, -2, "result");
#endif
  return 1;
}


/**
 *
 */
static int
es_entitydecode(duk_context *ctx)
{
  char *out = strdup(duk_safe_to_string(ctx, 0));
  html_entities_decode(out);
  duk_push_string(ctx, out);
  free(out);
  return 1;
}


/**
 *
 */
static int
es_queryStringSplit(duk_context *ctx)
{
  const char *str = duk_safe_to_string(ctx, 0);
  char *s0, *s;
  duk_push_object(ctx);

  s0 = s = strdup(str);

  while(s) {

    char *k = s;
    char *v = strchr(s, '=');
    if(v == NULL)
      break;

    *v++ = 0;

    if((s = strchr(v, '&')) != NULL)
      *s++ = 0;

    k = strdup(k);
    v = strdup(v);

    url_deescape(k);
    url_deescape(v);

    duk_push_string(ctx, v);
    duk_put_prop_string(ctx, -2, k);
    free(k);
    free(v);
  }
  free(s0);
  return 1;
}


/**
 *
 */
static int
es_escape(duk_context *ctx, int how)
{
  const char *str = duk_safe_to_string(ctx, 0);

  size_t len = url_escape(NULL, 0, str, how);
  char *r = malloc(len);
  url_escape(r, len, str, how);

  duk_push_lstring(ctx, r, len);
  free(r);
  return 1;
}

/**
 *
 */
static int
es_pathEscape(duk_context *ctx)
{
  return es_escape(ctx, URL_ESCAPE_PATH);
}


/**
 *
 */
static int
es_paramEscape(duk_context *ctx)
{
  return es_escape(ctx, URL_ESCAPE_PARAM);
}


/**
 * Showtime object exposed functions
 */
const duk_function_list_entry fnlist_Showtime_misc[] = {
  { "webpopup",              es_webpopup,         3 },
  { "entityDecode",          es_entitydecode,     1 },
  { "queryStringSplit",      es_queryStringSplit, 1 },
  { "pathEscape",            es_pathEscape,       1 },
  { "paramEscape",           es_paramEscape,      1 },
  { NULL, NULL, 0}
};
 