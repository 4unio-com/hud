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

#include "query.h"

struct _HudClientQueryPrivate {
	int dummy;
};

#define HUD_CLIENT_QUERY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_QUERY, HudClientQueryPrivate))

static void hud_client_query_class_init  (HudClientQueryClass *klass);
static void hud_client_query_init        (HudClientQuery *self);
static void hud_client_query_constructed (GObject *object);
static void hud_client_query_dispose     (GObject *object);
static void hud_client_query_finalize    (GObject *object);

G_DEFINE_TYPE (HudClientQuery, hud_client_query, G_TYPE_OBJECT);

static void
hud_client_query_class_init (HudClientQueryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientQueryPrivate));

	object_class->dispose = hud_client_query_dispose;
	object_class->finalize = hud_client_query_finalize;
	object_class->constructed = hud_client_query_constructed;

	return;
}

static void
hud_client_query_init (HudClientQuery *self)
{
	self->priv = HUD_CLIENT_QUERY_GET_PRIVATE(self);

	return;
}

static void
hud_client_query_constructed (GObject *object)
{
	G_OBJECT_CLASS (hud_client_query_parent_class)->constructed (object);

	return;
}

static void
hud_client_query_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_client_query_parent_class)->dispose (object);
	return;
}

static void
hud_client_query_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_client_query_parent_class)->finalize (object);
	return;
}

HudClientQuery *
hud_client_query_new (const gchar * query)
{

	return NULL;
}

HudClientQuery *
hud_client_query_new_for_connection (const gchar * query, HudClientConnection * connection)
{

	return NULL;
}

void
hud_client_query_set_query (HudClientQuery * cquery, const gchar * query)
{

	return;
}

const gchar *
hud_client_query_get_query (HudClientQuery * cquery)
{


	return NULL;
}

DeeModel *
hud_client_query_get_results_model (HudClientQuery * cquery)
{


	return NULL;
}
