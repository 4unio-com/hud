/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <tests/menus/FakeTouchApplication.h>
#include <tests/menus/TestAdaptor.h>

FakeTouchApplication::FakeTouchApplication(const QString &applicationId,
		const QDBusConnection &connection, const QString &dbusName,
		const QString &dbusPath) :
		m_adaptor(new TestAdaptor(this)), m_connection(connection), m_exportId(
				0) {

	if (!m_connection.registerObject(dbusPath, this)) {
		throw std::logic_error("Unable to register test object on DBus");
	}

	if (!m_connection.registerService(dbusName)) {
		throw std::logic_error("Unable to register test service on DBus");
	}

	GError *error = NULL;
	m_sessionBus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (error != NULL) {
		qWarning(
				"%s:\n"
				"\tCould not get session bus. Actions will not be available through D-Bus.\n"
				"\tReason: %s", __PRETTY_FUNCTION__,
				error->message);
		g_error_free(error);
		error = NULL;
	}

	m_hudManager = hud_manager_new(qPrintable(applicationId));

	QStringList actionNames;
	actionNames << "save" << "quiter" << "close" << "nothing";

	m_actionGroup = g_simple_action_group_new();
	for (const QString &name : actionNames) {
		GSimpleAction *action = g_simple_action_new(qPrintable(name), NULL);
		g_signal_connect(action, "activate", G_CALLBACK(action_callback), this);
		g_action_map_add_action(G_ACTION_MAP(m_actionGroup), G_ACTION(action));
	}

	/* create a new HUD context */
	m_actionPublisher = hud_action_publisher_new(
	HUD_ACTION_PUBLISHER_ALL_WINDOWS, "action_context_0");
	hud_action_publisher_add_action_group(m_actionPublisher, "hud",
			UNITY_ACTION_EXPORT_PATH);
	hud_manager_add_actions(m_hudManager, m_actionPublisher);

	hud_manager_switch_window_context(m_hudManager, m_actionPublisher);

	for (const QString &name : actionNames) {
		HudActionDescription *desc = hud_action_description_new(
				qPrintable(QString("hud.%1").arg(name)), NULL);
		hud_action_description_set_attribute_value(desc,
		G_MENU_ATTRIBUTE_LABEL, g_variant_new_string(qPrintable(name)));
		hud_action_description_set_attribute_value(desc, "description",
				g_variant_new_string(
				qPrintable(QString("%1 description").arg(name))));
//		hud_action_description_set_attribute_value(desc, "keywords",
//				g_variant_new_string(qPrintable(action->keywords())));
		hud_action_publisher_add_description(m_actionPublisher, desc);
	}

	if (m_sessionBus) {
		m_exportId = g_dbus_connection_export_action_group(m_sessionBus,
				UNITY_ACTION_EXPORT_PATH, G_ACTION_GROUP(m_actionGroup),
				&error);
		if (m_exportId == 0) {
			Q_ASSERT(error != NULL);
			qWarning(
					"%s:\n"
					"\tCould not export the main action group. Actions will not be available through D-Bus.\n"
					"\tReason: %s", __PRETTY_FUNCTION__,
					error->message);
			g_error_free(error);
			error = NULL;
		}
	}
}

FakeTouchApplication::~FakeTouchApplication() {
	g_dbus_connection_unexport_action_group(m_sessionBus, m_exportId);
	g_object_unref(m_actionPublisher);
	g_object_unref(m_actionGroup);
	g_object_unref(m_hudManager);
	g_object_unref(m_sessionBus);
}

void FakeTouchApplication::action_callback(GSimpleAction *simple,
G_GNUC_UNUSED GVariant *parameter, G_GNUC_UNUSED gpointer user_data) {
	FakeTouchApplication * self = static_cast<FakeTouchApplication *>(user_data);
	gchar *name = NULL;
	g_object_get(simple, "name", &name, NULL);
	self->m_adaptor->ActionInvoked(name);
	g_free(name);
}

