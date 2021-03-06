/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <exo/exo.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <libheartlenvutil/libheartlenvutil.h>

#include <common/panel-private.h>
#include <common/panel-dbus.h>
#include <common/panel-debug.h>

#include <libheartlenvpanel/libheartlenvpanel.h>
#include <libheartlenvpanel/xfce-panel-plugin-provider.h>

#include <panel/panel-module.h>
#include <panel/panel-plugin-external.h>
#include <panel/panel-plugin-external-wrapper.h>
#include <panel/panel-window.h>
#include <panel/panel-dialogs.h>
#include <panel/panel-marshal.h>



#define WRAPPER_BIN HELPERDIR G_DIR_SEPARATOR_S "wrapper"



static GObject   *panel_plugin_external_wrapper_constructor              (GType                           type,
                                                                          guint                           n_construct_params,
                                                                          GObjectConstructParam          *construct_params);
static void       panel_plugin_external_wrapper_set_properties           (PanelPluginExternal            *external,
                                                                          GSList                         *properties);
static gchar    **panel_plugin_external_wrapper_get_argv                 (PanelPluginExternal            *external,
                                                                          gchar                         **arguments);
static gboolean   panel_plugin_external_wrapper_remote_event             (PanelPluginExternal            *external,
                                                                          const gchar                    *name,
                                                                          const GValue                   *value,
                                                                          guint                          *handle);
static gboolean   panel_plugin_external_wrapper_dbus_provider_signal     (PanelPluginExternalWrapper     *external,
                                                                          XfcePanelPluginProviderSignal   provider_signal,
                                                                          GError                        **error);
static gboolean   panel_plugin_external_wrapper_dbus_remote_event_result (PanelPluginExternalWrapper     *external,
                                                                          guint                           handle,
                                                                          gboolean                        result,
                                                                          GError                        **error);



/* include the dbus glue generated by dbus-binding-tool */
#include <panel/panel-plugin-external-wrapper-infos.h>



struct _PanelPluginExternalWrapperClass
{
  PanelPluginExternalClass __parent__;
};

struct _PanelPluginExternalWrapper
{
  PanelPluginExternal __parent__;
};

enum
{
  SET,
  REMOTE_EVENT,
  REMOTE_EVENT_RESULT,
  LAST_SIGNAL
};



static guint external_signals[LAST_SIGNAL];



G_DEFINE_TYPE (PanelPluginExternalWrapper, panel_plugin_external_wrapper, PANEL_TYPE_PLUGIN_EXTERNAL)



static void
panel_plugin_external_wrapper_class_init (PanelPluginExternalWrapperClass *klass)
{
  GObjectClass             *gobject_class;
  PanelPluginExternalClass *plugin_external_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructor = panel_plugin_external_wrapper_constructor;

  plugin_external_class = PANEL_PLUGIN_EXTERNAL_CLASS (klass);
  plugin_external_class->get_argv = panel_plugin_external_wrapper_get_argv;
  plugin_external_class->set_properties = panel_plugin_external_wrapper_set_properties;
  plugin_external_class->remote_event = panel_plugin_external_wrapper_remote_event;

  external_signals[SET] =
    g_signal_new (g_intern_static_string ("set"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  PANEL_TYPE_DBUS_SET_SIGNAL);

  external_signals[REMOTE_EVENT] =
    g_signal_new (g_intern_static_string ("remote-event"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  _panel_marshal_VOID__STRING_BOXED_UINT,
                  G_TYPE_NONE, 3,
                  G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_UINT);

  external_signals[REMOTE_EVENT_RESULT] =
    g_signal_new (g_intern_static_string ("remote-event-result"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  _panel_marshal_VOID__UINT_BOOLEAN,
                  G_TYPE_NONE, 2,
                  G_TYPE_UINT, G_TYPE_BOOLEAN);

  /* add dbus type info for plugins */
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
      &dbus_glib_panel_plugin_external_wrapper_object_info);
}



static void
panel_plugin_external_wrapper_init (PanelPluginExternalWrapper *external)
{
}



static GObject *
panel_plugin_external_wrapper_constructor (GType                  type,
                                           guint                  n_construct_params,
                                           GObjectConstructParam *construct_params)
{
  GObject         *object;
  gchar           *path;
  DBusGConnection *connection;
  GError          *error = NULL;

  object = G_OBJECT_CLASS (panel_plugin_external_wrapper_parent_class)->constructor (type,
                                                                                     n_construct_params,
                                                                                     construct_params);

  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (G_LIKELY (connection != NULL))
    {
      /* register the object in dbus, the wrapper will monitor this object */
      panel_return_val_if_fail (PANEL_PLUGIN_EXTERNAL (object)->unique_id != -1, NULL);
      path = g_strdup_printf (PANEL_DBUS_WRAPPER_PATH, PANEL_PLUGIN_EXTERNAL (object)->unique_id);
      dbus_g_connection_register_g_object (connection, path, object);
      panel_debug (PANEL_DEBUG_EXTERNAL, "register dbus path %s", path);
      g_free (path);

      dbus_g_connection_unref (connection);
    }
  else
    {
      g_critical ("Failed to get D-Bus session bus: %s", error->message);
      g_error_free (error);
    }

  return object;
}



static gchar **
panel_plugin_external_wrapper_get_argv (PanelPluginExternal   *external,
                                        gchar               **arguments)
{
  guint   i, argc = PLUGIN_ARGV_ARGUMENTS;
  gchar **argv;

  panel_return_val_if_fail (PANEL_IS_PLUGIN_EXTERNAL_WRAPPER (external), NULL);
  panel_return_val_if_fail (PANEL_IS_MODULE (external->module), NULL);
  panel_return_val_if_fail (GTK_IS_SOCKET (external), NULL);

  /* add the number of arguments to the argc count */
  if (G_UNLIKELY (arguments != NULL))
    argc += g_strv_length (arguments);

  /* setup the basic argv */
  argv = g_new0 (gchar *, argc + 1);
  argv[PLUGIN_ARGV_0] = g_strdup (WRAPPER_BIN);
  argv[PLUGIN_ARGV_FILENAME] = g_strdup (panel_module_get_filename (external->module));
  argv[PLUGIN_ARGV_UNIQUE_ID] = g_strdup_printf ("%d", external->unique_id);;
  argv[PLUGIN_ARGV_SOCKET_ID] = g_strdup_printf ("%u", gtk_socket_get_id (GTK_SOCKET (external)));;
  argv[PLUGIN_ARGV_NAME] = g_strdup (panel_module_get_name (external->module));
  argv[PLUGIN_ARGV_DISPLAY_NAME] = g_strdup (panel_module_get_display_name (external->module));
  argv[PLUGIN_ARGV_COMMENT] = g_strdup (panel_module_get_comment (external->module));
  argv[PLUGIN_ARGV_BACKGROUND_IMAGE] = g_strdup (""); /* unused, for 4.6 plugins only */

  /* append the arguments */
  if (G_UNLIKELY (arguments != NULL))
    {
      for (i = 0; arguments[i] != NULL; i++)
        argv[i + PLUGIN_ARGV_ARGUMENTS] = g_strdup (arguments[i]);
    }

  return argv;
}



static void
panel_plugin_external_wrapper_set_properties (PanelPluginExternal *external,
                                              GSList              *properties)
{
  GPtrArray      *array;
  GValue          message = { 0, };
  PluginProperty *property;
  GSList         *li;
  guint           i;

  array = g_ptr_array_sized_new (1);

  g_value_init (&message, PANEL_TYPE_DBUS_SET_PROPERTY);
  g_value_take_boxed (&message, dbus_g_type_specialized_construct (G_VALUE_TYPE (&message)));

  /* put properties in a dbus-suitable array for the wrapper */
  for (li = properties; li != NULL; li = li->next)
    {
      property = li->data;

      dbus_g_type_struct_set (&message,
                              DBUS_SET_TYPE, property->type,
                              DBUS_SET_VALUE, &property->value,
                              G_MAXUINT);

      g_ptr_array_add (array, g_value_dup_boxed (&message));
    }

  /* send array to the wrapper */
  g_signal_emit (G_OBJECT (external), external_signals[SET], 0, array);

  for (i = 0; i < array->len; i++)
    g_value_array_free (g_ptr_array_index (array, i));
  g_ptr_array_free (array, TRUE);
  g_value_unset (&message);
}



static gboolean
panel_plugin_external_wrapper_remote_event (PanelPluginExternal *external,
                                            const gchar         *name,
                                            const GValue        *value,
                                            guint               *handle)
{
  static guint  handle_counter = 0;
  GValue        dummy_value = { 0, };
  const GValue *real_value = value;

  panel_return_val_if_fail (PANEL_IS_PLUGIN_EXTERNAL_WRAPPER (external), TRUE);
  panel_return_val_if_fail (XFCE_IS_PANEL_PLUGIN_PROVIDER (external), TRUE);
  panel_return_val_if_fail (value == NULL || G_IS_VALUE (value), FALSE);

  if (G_UNLIKELY (handle_counter > G_MAXUINT - 2))
    handle_counter = 0;
  *handle = ++handle_counter;

  if (value == NULL)
    {
      /* we send a dummy value over dbus */
      g_value_init (&dummy_value, G_TYPE_UCHAR);
      g_value_set_uchar (&dummy_value, '\0');
      real_value = &dummy_value;
    }

  g_signal_emit (G_OBJECT (external), external_signals[REMOTE_EVENT], 0,
                 name, real_value, *handle);

  if (real_value != value)
    g_value_unset (&dummy_value);

  return TRUE;
}



static gboolean
panel_plugin_external_wrapper_dbus_provider_signal (PanelPluginExternalWrapper     *external,
                                                    XfcePanelPluginProviderSignal   provider_signal,
                                                    GError                        **error)
{
  panel_return_val_if_fail (PANEL_IS_PLUGIN_EXTERNAL (external), FALSE);
  panel_return_val_if_fail (XFCE_IS_PANEL_PLUGIN_PROVIDER (external), FALSE);

  switch (provider_signal)
    {
    case PROVIDER_SIGNAL_SHOW_CONFIGURE:
      PANEL_PLUGIN_EXTERNAL (external)->show_configure = TRUE;
      break;

    case PROVIDER_SIGNAL_SHOW_ABOUT:
      PANEL_PLUGIN_EXTERNAL (external)->show_about = TRUE;
      break;

    default:
      /* other signals are handled in panel-applications.c */
      xfce_panel_plugin_provider_emit_signal (XFCE_PANEL_PLUGIN_PROVIDER (external),
                                              provider_signal);
      break;
    }

  return TRUE;
}



static gboolean
panel_plugin_external_wrapper_dbus_remote_event_result (PanelPluginExternalWrapper  *external,
                                                        guint                        handle,
                                                        gboolean                     result,
                                                        GError                     **error)
{
  panel_return_val_if_fail (PANEL_IS_PLUGIN_EXTERNAL (external), FALSE);

  g_signal_emit (G_OBJECT (external), external_signals[REMOTE_EVENT_RESULT], 0,
                 handle, result);

  return TRUE;
}



GtkWidget *
panel_plugin_external_wrapper_new (PanelModule  *module,
                                   gint          unique_id,
                                   gchar       **arguments)
{
  panel_return_val_if_fail (PANEL_IS_MODULE (module), NULL);
  panel_return_val_if_fail (unique_id != -1, NULL);

  return g_object_new (PANEL_TYPE_PLUGIN_EXTERNAL_WRAPPER,
                       "module", module,
                       "unique-id", unique_id,
                       "arguments", arguments, NULL);
}
