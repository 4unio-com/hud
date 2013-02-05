/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ted Gould <ted@canonical.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dee.h>

#include "query.h"
#include "connection.h"
#include "query-iface.h"

struct _HudClientQueryPrivate {
	_HudQueryComCanonicalHudQuery * proxy;
	HudClientConnection * connection;
	gchar * query;
	DeeModel * results;
	DeeModel * appstack;
};

#define HUD_CLIENT_QUERY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_QUERY, HudClientQueryPrivate))

enum {
	PROP_0 = 0,
	PROP_CONNECTION,
	PROP_QUERY,
};

#define PROP_CONNECTION_S  "connection"
#define PROP_QUERY_S       "query"

static void hud_client_query_class_init  (HudClientQueryClass *klass);
static void hud_client_query_init        (HudClientQuery *self);
static void hud_client_query_constructed (GObject *object);
static void hud_client_query_dispose     (GObject *object);
static void hud_client_query_finalize    (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);

G_DEFINE_TYPE (HudClientQuery, hud_client_query, G_TYPE_OBJECT);

static guint hud_client_query_signal_voice_query_loading;
static guint hud_client_query_signal_voice_query_failed;
static guint hud_client_query_signal_voice_query_listening;
static guint hud_client_query_signal_voice_query_heard_something;
static guint hud_client_query_signal_voice_query_finished;

static void
hud_client_query_class_init (HudClientQueryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientQueryPrivate));

	object_class->dispose = hud_client_query_dispose;
	object_class->finalize = hud_client_query_finalize;
	object_class->constructed = hud_client_query_constructed;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_CONNECTION,
	                                 g_param_spec_object(PROP_CONNECTION_S, "Connection to the HUD service",
	                                              "HUD service connection",
	                                              HUD_CLIENT_TYPE_CONNECTION,
	                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_QUERY,
	                                 g_param_spec_string(PROP_QUERY_S, "Query to the HUD service",
	                                              "HUD query",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/**
	 * HudClientQuery::voice-query-loading:
	 *
	 * The voice recognition toolkit is loading, and not ready for speech yet.
	 */
	hud_client_query_signal_voice_query_loading = g_signal_new (
		"voice-query-loading", HUD_CLIENT_TYPE_QUERY, G_SIGNAL_RUN_LAST, 0, NULL,
		NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

	/**
   * HudClientQuery::voice-query-failed:
   *
   * The voice recognition toolkit has failed to connect to the audio device.
   * The specific cause is provided as an argument.
   */
  hud_client_query_signal_voice_query_failed = g_signal_new (
    "voice-query-failed", HUD_CLIENT_TYPE_QUERY, G_SIGNAL_RUN_LAST, 0, NULL,
    NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 1, G_TYPE_STRING );

	/**
   * HudClientQuery::voice-query-listening:
   *
   * The voice recognition toolkit is active and listening for speech.
   */
	hud_client_query_signal_voice_query_listening = g_signal_new (
		"voice-query-listening", HUD_CLIENT_TYPE_QUERY, G_SIGNAL_RUN_LAST, 0,
		NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

	/**
   * HudClientQuery::voice-query-heard-something:
   *
   * The voice recognition toolkit has heard an utterance.
   */
  hud_client_query_signal_voice_query_heard_something = g_signal_new (
    "voice-query-heard-something", HUD_CLIENT_TYPE_QUERY, G_SIGNAL_RUN_LAST,
    0, NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

	/**
   * HudClientQuery::voice-query-finished:
   *
   * The voice recognition toolkit has completed and has a (possibly empty) result.
   */
	hud_client_query_signal_voice_query_finished = g_signal_new (
		"voice-query-finished", HUD_CLIENT_TYPE_QUERY, G_SIGNAL_RUN_LAST, 0, NULL,
		NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 1, G_TYPE_STRING );

	return;
}

static void
hud_client_query_init (HudClientQuery *self)
{
	self->priv = HUD_CLIENT_QUERY_GET_PRIVATE(self);

	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(obj);

	switch (id) {
	case PROP_CONNECTION:
		g_clear_object(&self->priv->connection);
		self->priv->connection = g_value_dup_object(value);
		break;
	case PROP_QUERY:
		hud_client_query_set_query(self, g_value_get_string(value));
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(obj);

	switch (id) {
	case PROP_CONNECTION:
		g_value_set_object(value, self->priv->connection);
		break;
	case PROP_QUERY:
		g_value_set_string(value, self->priv->query);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
hud_client_query_voice_query_loading (_HudQueryComCanonicalHudQuery *object, gpointer user_data)
{
	g_signal_emit(user_data, hud_client_query_signal_voice_query_loading, 0);
}

static void
hud_client_query_voice_query_failed (_HudQueryComCanonicalHudQuery *object, const gchar *arg_cause, gpointer user_data)
{
  g_signal_emit (user_data, hud_client_query_signal_voice_query_failed,
      g_quark_try_string (arg_cause), arg_cause);
}

static void
hud_client_query_voice_query_listening (_HudQueryComCanonicalHudQuery *object, gpointer user_data)
{
	g_signal_emit(user_data, hud_client_query_signal_voice_query_listening, 0);
}

static void
hud_client_query_voice_query_heard_something (_HudQueryComCanonicalHudQuery *object, gpointer user_data)
{
  g_signal_emit(user_data, hud_client_query_signal_voice_query_heard_something, 0);
}

static void
hud_client_query_voice_query_finished (_HudQueryComCanonicalHudQuery *object, const gchar *arg_query, gpointer user_data)
{
	g_signal_emit (user_data, hud_client_query_signal_voice_query_finished,
	g_quark_try_string (arg_query), arg_query);
}

static void
hud_client_query_constructed (GObject *object)
{
	HudClientQuery * cquery = HUD_CLIENT_QUERY(object);

	G_OBJECT_CLASS (hud_client_query_parent_class)->constructed (object);

	if (cquery->priv->connection == NULL) {
		cquery->priv->connection = hud_client_connection_get_ref();
	}

	if(cquery->priv->query == NULL) {
		cquery->priv->query = g_strdup("");
	}

	gchar * path = NULL;
	gchar * results = NULL;
	gchar * appstack = NULL;

	/* This is perhaps a little extreme, but really, if this is failing
	 there's a whole world of hurt for us. */
	g_return_if_fail(hud_client_connection_new_query(cquery->priv->connection, cquery->priv->query, &path, &results, &appstack));

	cquery->priv->proxy = _hud_query_com_canonical_hud_query_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_NONE,
		hud_client_connection_get_address(cquery->priv->connection),
		path,
		NULL, /* GCancellable */
		NULL  /* GError */
	);

	g_clear_object(&cquery->priv->results);
	cquery->priv->results = dee_shared_model_new(results);

	g_clear_object(&cquery->priv->appstack);
	cquery->priv->appstack = dee_shared_model_new(appstack);

	g_free(path);
	g_free(results);
	g_free(appstack);

	g_signal_connect_object (cquery->priv->proxy, "voice-query-loading",
		G_CALLBACK (hud_client_query_voice_query_loading), object, 0);
	g_signal_connect_object (cquery->priv->proxy, "voice-query-failed",
	    G_CALLBACK (hud_client_query_voice_query_failed), object, 0);
	g_signal_connect_object (cquery->priv->proxy, "voice-query-listening",
		G_CALLBACK (hud_client_query_voice_query_listening), object, 0);
	g_signal_connect_object (cquery->priv->proxy, "voice-query-heard-something",
	    G_CALLBACK (hud_client_query_voice_query_heard_something), object, 0);
	g_signal_connect_object (cquery->priv->proxy, "voice-query-finished",
		G_CALLBACK (hud_client_query_voice_query_finished), object, 0);
}

static void
hud_client_query_dispose (GObject *object)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(object);

	_hud_query_com_canonical_hud_query_call_close_query_sync(self->priv->proxy, NULL, NULL);

	g_clear_object(&self->priv->results);
	g_clear_object(&self->priv->appstack);
	g_clear_object(&self->priv->proxy);
	g_clear_object(&self->priv->connection);

	G_OBJECT_CLASS (hud_client_query_parent_class)->dispose (object);
	return;
}

static void
hud_client_query_finalize (GObject *object)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(object);

	g_clear_pointer(&self->priv->query, g_free);

	G_OBJECT_CLASS (hud_client_query_parent_class)->finalize (object);
	return;
}

/**
 * hud_client_query_new:
 * @query: String to build the initial set of results from
 *
 * Startes a query with the HUD using a specific string.  This
 * will block until the query is created.
 *
 * Return value: (transfer full): A new #HudClientQuery object
 */
HudClientQuery *
hud_client_query_new (const gchar * query)
{
	return HUD_CLIENT_QUERY(g_object_new(HUD_CLIENT_TYPE_QUERY,
		PROP_QUERY_S, query,
		NULL
	));
}

/**
 * hud_client_query_new_for_connection:
 * @query: String to build the initial set of results from
 * @connection: A custom #HudClientConnection to a non-default HUD service
 *
 * Very similar to hud_client_query_new() except that it uses a
 * custom connection.  This is mostly for testing, though it is
 * available if you need it.
 *
 * Return value: (transfer full): A new #HudClientQuery object
 */
HudClientQuery *
hud_client_query_new_for_connection (const gchar * query, HudClientConnection * connection)
{
	return HUD_CLIENT_QUERY(g_object_new(HUD_CLIENT_TYPE_QUERY,
		PROP_CONNECTION_S, connection,
		PROP_QUERY_S, query,
		NULL
	));
}

/**
 * hud_client_query_set_query:
 * @cquery: A #HudClientQuery
 * @query: New query string
 *
 * This revises the query to be the new query string.  Updates can
 * be seen through the #DeeModel's.
 */
void
hud_client_query_set_query (HudClientQuery * cquery, const gchar * query)
{
	g_return_if_fail(HUD_CLIENT_IS_QUERY(cquery));

	g_clear_pointer(&cquery->priv->query, g_free);
	cquery->priv->query = g_strdup(query);

	if (cquery->priv->proxy != NULL) {
		gint revision = 0;
		_hud_query_com_canonical_hud_query_call_update_query_sync(cquery->priv->proxy, cquery->priv->query, &revision, NULL, NULL);
	}

	g_object_notify(G_OBJECT(cquery), PROP_QUERY_S);

	return;
}

/**
 * hud_client_query_get_query:
 * @cquery: A #HudClientQuery
 * 
 * Accessor for the current query string.
 *
 * Return value: (transfer none): Query string
 */
const gchar *
hud_client_query_get_query (HudClientQuery * cquery)
{
	g_return_val_if_fail(HUD_CLIENT_IS_QUERY(cquery), NULL);

	return cquery->priv->query;
}

static void
hud_client_query_voice_query_callback (GObject *source, GAsyncResult *result, gpointer user_data)
{
	g_assert(HUD_CLIENT_IS_QUERY(user_data));
	HudClientQuery *cquery = HUD_CLIENT_QUERY(user_data);
	gint revision = 0;
	gchar *query = NULL;
	GError *error = NULL;
	if (!_hud_query_com_canonical_hud_query_call_voice_query_finish (cquery->priv->proxy, &revision, &query, result, &error))
	{
		g_warning("Voice query failed to finish: [%s]", error->message);
		g_error_free(error);
		return;
	}

	g_clear_pointer(&cquery->priv->query, g_free);
	cquery->priv->query = query;
	g_object_notify (G_OBJECT(cquery), PROP_QUERY_S);
}

/**
 * hud_client_query_voice_query:
 * @cquery: A #HudClientQuery
 *
 * Execute a HUD query using voice recognition.
 *
 * Will cause a series of signals to be emitted indicating progress:
 * - voice-query-loading - the voice recognition toolkit is loading.
 * - voice-query-failed - the voice recognition toolkit has failed to initialize.
 * - voice-query-listening - the voice recognition toolkit is listening to speech.
 * - voice-query-heard-something - the voice recognition toolkit has heard a complete utterance.
 * - voice-query-finished - the voice recognition toolkit has completed, and has a (possibly empty) result.
 */
void
hud_client_query_voice_query (HudClientQuery * cquery)
{
	g_return_if_fail(HUD_CLIENT_IS_QUERY(cquery));

	if (cquery->priv->proxy != NULL) {
		g_debug("Running voice query");
		_hud_query_com_canonical_hud_query_call_voice_query (cquery->priv->proxy, NULL, hud_client_query_voice_query_callback, cquery);
	}
}

/**
 * hud_client_query_get_results_model:
 * @cquery: A #HudClientQuery
 *
 * Accessor for the current results model.
 *
 * Return value: (transfer none): Results Model
 */
DeeModel *
hud_client_query_get_results_model (HudClientQuery * cquery)
{
	g_return_val_if_fail(HUD_CLIENT_IS_QUERY(cquery), NULL);

	return cquery->priv->results;
}

/**
 * hud_client_query_get_appstack_model:
 * @cquery: A #HudClientQuery
 *
 * Accessor for the current appstack model.
 *
 * Return value: (transfer none): Appstack Model
 */
DeeModel *
hud_client_query_get_appstack_model (HudClientQuery * cquery)
{
	g_return_val_if_fail(HUD_CLIENT_IS_QUERY(cquery), NULL);

	return cquery->priv->appstack;
}

/**
 * hud_client_query_set_appstack_app:
 * @cquery: A #HudClientQuery
 * @application_id: New application to get results from
 *
 * This revises the query application to be application_id.  Updates can
 * be seen through the #DeeModel's.
 */
void
hud_client_query_set_appstack_app (HudClientQuery *        cquery,
                                   const gchar *           application_id)
{
	g_return_if_fail(HUD_CLIENT_IS_QUERY(cquery));

	if (cquery->priv->proxy != NULL) {
		gint revision = 0;
		_hud_query_com_canonical_hud_query_call_update_app_sync(cquery->priv->proxy, application_id, &revision, NULL, NULL);
	}

	return;
}

/**
 * hud_client_query_execute_command:
 * @cquery: A #HudClientQuery
 * @command_key: The key from the results model for the entry to activate
 * @timestamp: Timestamp for the user event
 *
 * Executes a particular entry from the results model.  The @command_key
 * should be grabbed from the table and passed to this function to activate
 * it.  This function will block until the command is activated.
 */
void
hud_client_query_execute_command (HudClientQuery * cquery, GVariant * command_key, guint timestamp)
{
	g_return_if_fail(HUD_CLIENT_IS_QUERY(cquery));
	g_return_if_fail(command_key != NULL);

	_hud_query_com_canonical_hud_query_call_execute_command_sync(cquery->priv->proxy, command_key, timestamp, NULL, NULL);

	return;
}

/**
 * hud_client_query_execute_param_command:
 * @cquery: A #HudClientQuery
 * @command_key: The key from the results model for the entry to activate
 * @timestamp: Timestamp for the user event
 *
 * Executes a command that results in a parameterized dialog
 * which is controlled using the returned #HudClientParam object.
 * When created this sends the "opened" event to the application.
 *
 * Return Value: (transfer full): Object to control the parameterized dialog.
 */
HudClientParam *
hud_client_query_execute_param_command (HudClientQuery * cquery, GVariant * command_key, guint timestamp)
{
	g_return_val_if_fail(HUD_CLIENT_IS_QUERY(cquery), NULL);
	g_return_val_if_fail(command_key != NULL, NULL);

	gchar * sender = g_dbus_proxy_get_name_owner(G_DBUS_PROXY(cquery->priv->proxy));
	gchar * base_action = NULL;
	gchar * action_path = NULL;
	gchar * model_path = NULL;
	gint section = 0;

	_hud_query_com_canonical_hud_query_call_execute_parameterized_sync(cquery->priv->proxy, command_key, timestamp, &base_action, &action_path, &model_path, &section, NULL, NULL);

	HudClientParam * param = hud_client_param_new(sender, base_action, action_path, model_path, section);

	g_free(sender);
	g_free(base_action);
	g_free(action_path);
	g_free(model_path);

	return param;
}
