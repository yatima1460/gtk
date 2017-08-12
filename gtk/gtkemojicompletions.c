/* gtkemojicompletions.c: An Emoji picker widget
 * Copyright 2017, Red Hat, Inc.
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

#include "gtkemojicompletions.h"

#include "gtkbox.h"
#include "gtkcssprovider.h"
#include "gtklistbox.h"
#include "gtklabel.h"
#include "gtkpopover.h"
#include "gtkintl.h"
#include "gtkprivate.h"

struct _GtkEmojiCompletions
{
  GtkPopover parent_instance;

  GtkWidget *list;

  GVariant *data;
};

struct _GtkEmojiCompletionsClass {
  GtkPopoverClass parent_class;
};

enum {
  EMOJI_PICKED,
  LAST_SIGNAL
};

static int signals[LAST_SIGNAL];

G_DEFINE_TYPE (GtkEmojiCompletions, gtk_emoji_completions, GTK_TYPE_POPOVER)

static void
gtk_emoji_completions_finalize (GObject *object)
{
  //GtkEmojiCompletions *completions = GTK_EMOJI_COMPLETIONS (object);

  //g_variant_unref (completions->data);

  G_OBJECT_CLASS (gtk_emoji_completions_parent_class)->finalize (object);
}

static void
emoji_activated (GtkListBox    *list,
                 GtkListBoxRow *row,
                 gpointer       data)
{
  GtkEmojiCompletions *completions = data;
  const char *text;

  gtk_popover_popdown (GTK_POPOVER (completions));

  text = (const char *)g_object_get_data (G_OBJECT (row), "text");

  g_signal_emit (data, signals[EMOJI_PICKED], 0, text);
}

static void
add_emoji (GtkWidget    *list,
           GVariant     *item)
{
  GtkWidget *child;
  GtkWidget *label;
  GtkWidget *box;
  PangoAttrList *attrs;
  GVariant *codes;
  char text[64];
  char *p = text;
  int i;
  char *tmp;

  codes = g_variant_get_child_value (item, 0);
  for (i = 0; i < g_variant_n_children (codes); i++)
    {
      gunichar code;

      g_variant_get_child (codes, i, "u", &code);
      if (code != 0)
        p += g_unichar_to_utf8 (code, p);
    }
   p[0] = 0;

  label = gtk_label_new (text);
  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_X_LARGE));
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);

  child = gtk_list_box_row_new ();
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (child), box);
  gtk_box_pack_start (GTK_BOX (box), label);

  tmp = g_strdup_printf (":%s:", "grinning_face");
  label = gtk_label_new (tmp);
  g_free (tmp);
  gtk_box_pack_start (GTK_BOX (box), label);

  g_object_set_data_full (G_OBJECT (child), "text", g_strdup (text), g_free);
  gtk_style_context_add_class (gtk_widget_get_style_context (child), "emoji-completion-row");
  g_object_set_data_full (G_OBJECT (child), "text", g_strdup (text), g_free);

  gtk_list_box_insert (GTK_LIST_BOX (list), child, -1);
}

static void
populate_emoji_completions (GtkEmojiCompletions *completions)
{
  g_autoptr(GBytes) bytes = NULL;
  GVariantIter iter;
  GVariant *item;
  int i;

  bytes = g_resources_lookup_data ("/org/gtk/libgtk/emoji/emoji.data", 0, NULL);
  completions->data = g_variant_ref_sink (g_variant_new_from_bytes (G_VARIANT_TYPE ("a(aus)"), bytes, TRUE));

  g_variant_iter_init (&iter, completions->data);
  i = 0;
  while ((item = g_variant_iter_next_value (&iter)))
    {
      add_emoji (completions->list, item);
      i++;
      if (i == 5)
        break;
    }
}

static void
gtk_emoji_completions_init (GtkEmojiCompletions *completions)
{
  gtk_widget_init_template (GTK_WIDGET (completions));

  populate_emoji_completions (completions);
}

static void
gtk_emoji_completions_class_init (GtkEmojiCompletionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtk_emoji_completions_finalize;

  signals[EMOJI_PICKED] = g_signal_new ("emoji-picked",
                                        G_OBJECT_CLASS_TYPE (object_class),
                                        G_SIGNAL_RUN_LAST,
                                        0,
                                        NULL, NULL,
                                        NULL,
                                        G_TYPE_NONE, 1, G_TYPE_STRING|G_SIGNAL_TYPE_STATIC_SCOPE);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gtk/libgtk/ui/gtkemojicompletions.ui");

  gtk_widget_class_bind_template_child (widget_class, GtkEmojiCompletions, list);

  gtk_widget_class_bind_template_callback (widget_class, emoji_activated);
}

GtkWidget *
gtk_emoji_completions_new (GtkWidget *widget)
{
  return GTK_WIDGET (g_object_new (GTK_TYPE_EMOJI_COMPLETIONS,
                                   "relative-to", widget,
                                   NULL));
}

gboolean
gtk_emoji_completions_show (GtkEmojiCompletions *completions,
                            const char          *text)
{
  gtk_popover_popup (GTK_POPOVER (completions));
  return TRUE;
}
