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

#ifndef __GSK_SL_TOKEN_STREAM_PRIVATE_H__
#define __GSK_SL_TOKEN_STREAM_PRIVATE_H__

#include <glib.h>

#include "gskslnodeprivate.h"
#include "gsksltokenizerprivate.h"

G_BEGIN_DECLS

typedef struct _GskSlTokenStream GskSlTokenStream;

GskSlTokenStream *      gsk_sl_token_stream_new                 (GBytes              *source);

GskSlTokenStream *      gsk_sl_token_stream_ref                 (GskSlTokenStream    *stream);
void                    gsk_sl_token_stream_unref               (GskSlTokenStream    *stream);

const GskSlToken *      gsk_sl_token_stream_get                 (GskSlTokenStream    *stream);
const GskCodeLocation * gsk_sl_token_stream_get_location        (GskSlTokenStream    *stream);
void                    gsk_sl_token_stream_consume             (GskSlTokenStream    *stream,
                                                                 GskSlNode           *consumer);

void                    gsk_sl_token_stream_error               (GskSlTokenStream    *stream,
                                                                 const char          *format,
                                                                 ...) G_GNUC_PRINTF(2, 3);

G_END_DECLS

#endif /* __GSK_SL_TOKEN_STREAM_PRIVATE_H__ */
