/*
 * Copyright (c) 2007-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <exo/exo.h>

#include "clock.h"
#include "clock-fuzzy.h"



static void     xfce_clock_fuzzy_set_property (GObject               *object,
                                               guint                  prop_id,
                                               const GValue          *value,
                                               GParamSpec            *pspec);
static void     xfce_clock_fuzzy_get_property (GObject               *object,
                                               guint                  prop_id,
                                               GValue                *value,
                                               GParamSpec            *pspec);
static void     xfce_clock_fuzzy_finalize     (GObject               *object);
static gboolean xfce_clock_fuzzy_update       (gpointer               user_data);



enum
{
  FUZZINESS_5_MINS = 0,
  FUZZINESS_15_MINS,
  FUZZINESS_DAY,

  FUZZINESS_MIN = FUZZINESS_5_MINS,
  FUZZINESS_MAX = FUZZINESS_DAY,
  FUZZINESS_DEFAULT = FUZZINESS_5_MINS
};

enum
{
  PROP_0,
  PROP_FUZZINESS,
  PROP_SIZE_RATIO,
  PROP_ORIENTATION
};

struct _XfceClockFuzzyClass
{
 GtkLabelClass __parent__;
};

struct _XfceClockFuzzy
{
  GtkLabel __parent__;

  ClockPluginTimeout *timeout;

  guint               fuzziness;
};

static const gchar *i18n_day_sectors[] =
{
  N_("Night"),
  N_("Early morning"),
  N_("Morning"),
  N_("Almost noon"),
  N_("Noon"),
  N_("Afternoon"),
  N_("Evening"),
  N_("Late evening")
};

static const gchar *i18n_hour_sectors[] =
{
  /* I18N: %0 will be replaced with the preceding hour, %1 with
   * the comming hour */
  /* xgettext:no-c-format */ N_("%0 o'clock"),
  /* xgettext:no-c-format */ N_("five past %0"),
  /* xgettext:no-c-format */ N_("ten past %0"),
  /* xgettext:no-c-format */ N_("quarter past %0"),
  /* xgettext:no-c-format */ N_("twenty past %0"),
  /* xgettext:no-c-format */ N_("twenty five past %0"),
  /* xgettext:no-c-format */ N_("half past %0"),
  /* xgettext:no-c-format */ N_("twenty five to %1"),
  /* xgettext:no-c-format */ N_("twenty to %1"),
  /* xgettext:no-c-format */ N_("quarter to %1"),
  /* xgettext:no-c-format */ N_("ten to %1"),
  /* xgettext:no-c-format */ N_("five to %1"),
  /* xgettext:no-c-format */ N_("%1 o'clock")
};

static const gchar *i18n_hour_sectors_one[] =
{
  /* I18N: some languages have a singular form for the first hour,
   * other languages should just use the same strings as above */
  /* xgettext:no-c-format */ NC_("one", "%0 o'clock"),
  /* xgettext:no-c-format */ NC_("one", "five past %0"),
  /* xgettext:no-c-format */ NC_("one", "ten past %0"),
  /* xgettext:no-c-format */ NC_("one", "quarter past %0"),
  /* xgettext:no-c-format */ NC_("one", "twenty past %0"),
  /* xgettext:no-c-format */ NC_("one", "twenty five past %0"),
  /* xgettext:no-c-format */ NC_("one", "half past %0"),
  /* xgettext:no-c-format */ NC_("one", "twenty five to %1"),
  /* xgettext:no-c-format */ NC_("one", "twenty to %1"),
  /* xgettext:no-c-format */ NC_("one", "quarter to %1"),
  /* xgettext:no-c-format */ NC_("one", "ten to %1"),
  /* xgettext:no-c-format */ NC_("one", "five to %1"),
  /* xgettext:no-c-format */ NC_("one", "%1 o'clock")
};

static const gchar *i18n_hour_names[] =
{
  N_("one"),
  N_("two"),
  N_("three"),
  N_("four"),
  N_("five"),
  N_("six"),
  N_("seven"),
  N_("eight"),
  N_("nine"),
  N_("ten"),
  N_("eleven"),
  N_("twelve")
};



XFCE_PANEL_DEFINE_TYPE (XfceClockFuzzy, xfce_clock_fuzzy, GTK_TYPE_LABEL)



static void
xfce_clock_fuzzy_class_init (XfceClockFuzzyClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = xfce_clock_fuzzy_set_property;
  gobject_class->get_property = xfce_clock_fuzzy_get_property;
  gobject_class->finalize = xfce_clock_fuzzy_finalize;

  g_object_class_install_property (gobject_class,
                                   PROP_SIZE_RATIO,
                                   g_param_spec_double ("size-ratio", NULL, NULL,
                                                        -1, G_MAXDOUBLE, -1.00,
                                                        G_PARAM_READABLE
                                                        | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation", NULL, NULL,
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      G_PARAM_WRITABLE
                                                      | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_FUZZINESS,
                                   g_param_spec_uint ("fuzziness", NULL, NULL,
                                                      FUZZINESS_MIN,
                                                      FUZZINESS_MAX,
                                                      FUZZINESS_DEFAULT,
                                                      G_PARAM_READWRITE
                                                      | G_PARAM_STATIC_STRINGS));
}



static void
xfce_clock_fuzzy_init (XfceClockFuzzy *fuzzy)
{
  fuzzy->fuzziness = FUZZINESS_DEFAULT;
  fuzzy->timeout = clock_plugin_timeout_new (CLOCK_INTERVAL_MINUTE,
                                             xfce_clock_fuzzy_update,
                                             fuzzy);

  gtk_label_set_justify (GTK_LABEL (fuzzy), GTK_JUSTIFY_CENTER);
}



static void
xfce_clock_fuzzy_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  XfceClockFuzzy *fuzzy = XFCE_CLOCK_FUZZY (object);
  guint           fuzziness;

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      gtk_label_set_angle (GTK_LABEL (object),
          g_value_get_enum (value) == GTK_ORIENTATION_HORIZONTAL ?
          0 : 270);
      break;

    case PROP_FUZZINESS:
      fuzziness = g_value_get_uint (value);
      if (G_LIKELY (fuzzy->fuzziness != fuzziness))
        {
          fuzzy->fuzziness = fuzziness;
          xfce_clock_fuzzy_update (fuzzy);
        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_clock_fuzzy_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  XfceClockFuzzy *fuzzy = XFCE_CLOCK_FUZZY (object);

  switch (prop_id)
    {
    case PROP_FUZZINESS:
      g_value_set_uint (value, fuzzy->fuzziness);
      break;

    case PROP_SIZE_RATIO:
      g_value_set_double (value, -1.0);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_clock_fuzzy_finalize (GObject *object)
{
  /* stop the timeout */
  clock_plugin_timeout_free (XFCE_CLOCK_FUZZY (object)->timeout);

  (*G_OBJECT_CLASS (xfce_clock_fuzzy_parent_class)->finalize) (object);
}



static gboolean
xfce_clock_fuzzy_update (gpointer user_data)
{
  XfceClockFuzzy *fuzzy = XFCE_CLOCK_FUZZY (user_data);
  struct tm       tm;
  gint            sector;
  gint            minute, hour;
  gchar          *string;
  const gchar    *time_format;
  gchar          *p;
  gchar           pattern[3];

  panel_return_val_if_fail (XFCE_CLOCK_IS_FUZZY (fuzzy), FALSE);

  /* get the local time */
  clock_plugin_get_localtime (&tm);

  if (fuzzy->fuzziness == FUZZINESS_5_MINS
      || fuzzy->fuzziness == FUZZINESS_15_MINS)
    {
      /* set the time */
      minute = tm.tm_min;
      hour = tm.tm_hour;
      sector = 0;

      /* get the hour sector */
      if (fuzzy->fuzziness == FUZZINESS_5_MINS)
        {
          if (minute > 2)
            sector = (minute - 3) / 5 + 1;
        }
      else
        {
          if (minute > 6)
            sector = ((minute - 7) / 15 + 1) * 3;
        }

      /* translated time string */
      time_format = _(i18n_hour_sectors[sector]);

      /* add hour offset (%0 or %1 on the string) */
      p = strchr (time_format, '%');
      panel_assert (p != NULL && g_ascii_isdigit (*(p + 1)));
      if (G_LIKELY (p != NULL))
        hour += g_ascii_digit_value (*(p + 1));

      if (hour % 12 > 0)
        hour = hour % 12 - 1;
      else
        hour = 12 - hour % 12 - 1;

      if (hour == 0)
        {
          /* get the singular form of the format */
          time_format = _(i18n_hour_sectors_one[sector]);

          /* make sure we have to correct digit for the replace pattern */
          p = strchr (time_format, '%');
          panel_assert (p != NULL && g_ascii_isdigit (*(p + 1)));
        }

      /* replace the %? with the hour name */
      g_snprintf (pattern, sizeof (pattern), "%%%c", p != NULL ? *(p + 1) : '0');
      string = exo_str_replace (time_format, pattern, _(i18n_hour_names[hour]));
      gtk_label_set_text (GTK_LABEL (fuzzy), string);
      g_free (string);
    }
  else /* FUZZINESS_DAY */
    {
      gtk_label_set_text (GTK_LABEL (fuzzy), _(i18n_day_sectors[tm.tm_hour / 3]));
    }

  return TRUE;
}




GtkWidget *
xfce_clock_fuzzy_new (void)
{
  return g_object_new (XFCE_CLOCK_TYPE_FUZZY, NULL);
}
