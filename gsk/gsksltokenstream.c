/* GTK - The GIMP Toolkit
 *   
 * Copyright © 2017 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "gsksltokenstreamprivate.h"

#include "gsksltokenizerprivate.h"

struct _GskSlTokenStream
{
  int ref_count;

  GskSlTokenizer *tokenizer;
  GskCodeLocation location;
  GskSlToken token;
};

/* API */

static void
gsk_sl_token_stream_error_func (GskSlTokenizer        *parser,
                                gboolean               fatal,
                                const GskCodeLocation *location,
                                const GskSlToken      *token,
                                const GError          *error,
                                gpointer               user_data)
{
  g_print ("%3zu:%2zu: error: %3u: %s: %s\n",
           location->lines + 1, location->line_bytes,
           token->type, gsk_sl_token_to_string (token),
           error->message);
}

GskSlTokenStream *
gsk_sl_token_stream_new (GBytes *source)
{
  GskSlTokenStream *stream;

  stream = g_slice_new0 (GskSlTokenStream);

  stream->ref_count = 1;
  stream->tokenizer = gsk_sl_tokenizer_new (source, 
                                            gsk_sl_token_stream_error_func,
                                            stream,
                                            NULL);

  return stream;
}

GskSlTokenStream *
gsk_sl_token_stream_ref (GskSlTokenStream *stream)
{
  g_return_val_if_fail (stream != NULL, NULL);

  stream->ref_count += 1;

  return stream;
}

void
gsk_sl_token_stream_unref (GskSlTokenStream *stream)
{
  if (stream == NULL)
    return;

  stream->ref_count -= 1;
  if (stream->ref_count > 0)
    return;

  gsk_sl_tokenizer_unref (stream->tokenizer);
  gsk_sl_token_clear (&stream->token);

  g_slice_free (GskSlTokenStream, stream);
}

static gboolean
gsk_sl_token_is_skipped (const GskSlToken *token)
{
  return gsk_sl_token_is (token, GSK_SL_TOKEN_ERROR)
      || gsk_sl_token_is (token, GSK_SL_TOKEN_WHITESPACE)
      || gsk_sl_token_is (token, GSK_SL_TOKEN_COMMENT)
      || gsk_sl_token_is (token, GSK_SL_TOKEN_SINGLE_LINE_COMMENT);
}

static void
gsk_sl_token_ensure (GskSlTokenStream *stream)
{
  if (!gsk_sl_token_is (&stream->token, GSK_SL_TOKEN_EOF))
    return;

  do 
    {
      stream->location = *gsk_sl_tokenizer_get_location (stream->tokenizer);
      gsk_sl_tokenizer_read_token (stream->tokenizer, &stream->token);
    }
  while (gsk_sl_token_is_skipped (&stream->token));
}

const GskSlToken *
gsk_sl_token_stream_get (GskSlTokenStream *stream)
{
  gsk_sl_token_ensure (stream);

  return &stream->token;
}

const GskCodeLocation *
gsk_sl_token_stream_get_location (GskSlTokenStream *stream)
{
  gsk_sl_token_ensure (stream);

  return &stream->location;
}

void
gsk_sl_token_stream_consume (GskSlTokenStream *stream,
                             GskSlNode        *consumer)
{
  gsk_sl_token_ensure (stream);

  gsk_sl_token_clear (&stream->token);
}

void
gsk_sl_token_stream_error (GskSlTokenStream    *stream,
                           const char          *format,
                           ...)
{
  GError *error;
  va_list args;

  va_start (args, format);
  error = g_error_new_valist (G_FILE_ERROR,
                              G_FILE_ERROR_FAILED,
                              format,
                              args);
  va_end (args);

  gsk_sl_token_ensure (stream);
  gsk_sl_token_stream_error_func (stream->tokenizer,
                                  TRUE,
                                  &stream->location,
                                  &stream->token,
                                  error,
                                  NULL);

  g_error_free (error);
}
