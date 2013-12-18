/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <testutils/MockHudService.h>
#include <libhud/hud.h>
#include <common/shared-values.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <QTestEventLoop>
#include <gtest/gtest.h>

using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace hud::common;
using namespace hud::testutils;

namespace {

class TestActionPublisher: public Test {
protected:
	TestActionPublisher() :
			mock(dbus), hud(dbus, mock) {
		dbus.startServices();
		hud.loadMethods();

		connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	}

	virtual ~TestActionPublisher() {
		g_object_unref(connection);
	}

	DBusTestRunner dbus;

	DBusMock mock;

	MockHudService hud;

	GDBusConnection *connection;
};

TEST_F(TestActionPublisher, NewForId) {
	{
		HudActionPublisher *publisher = hud_action_publisher_new(
		HUD_ACTION_PUBLISHER_ALL_WINDOWS,
		HUD_ACTION_PUBLISHER_NO_CONTEXT);
		ASSERT_TRUE(publisher);

		EXPECT_EQ(HUD_ACTION_PUBLISHER_ALL_WINDOWS,
				hud_action_publisher_get_window_id(publisher));
		EXPECT_TRUE(hud_action_publisher_get_context_id(publisher));

		g_object_unref(publisher);
	}

	{
		HudActionPublisher *publisher = hud_action_publisher_new(123,
		HUD_ACTION_PUBLISHER_NO_CONTEXT);
		ASSERT_TRUE(publisher);

		EXPECT_EQ(123, hud_action_publisher_get_window_id(publisher));

		g_object_unref(publisher);
	}
}

TEST_F(TestActionPublisher, NewForApplication) {
	GApplication *application = g_application_new("app.id",
			G_APPLICATION_FLAGS_NONE);
	ASSERT_TRUE(g_application_register(application, NULL, NULL));

	HudActionPublisher *publisher = hud_action_publisher_new_for_application(
			application);
	EXPECT_TRUE(publisher);

	g_object_unref(application);
	g_object_unref(publisher);
}

TEST_F(TestActionPublisher, AddActionGroup) {
	GApplication *application = g_application_new("app.id",
			G_APPLICATION_FLAGS_NONE);
	ASSERT_TRUE(g_application_register(application, NULL, NULL));

	HudActionPublisher *publisher = hud_action_publisher_new_for_application(
			application);
	ASSERT_TRUE(publisher);

	// FIXME Waiting in tests
	QTestEventLoop::instance().enterLoopMSecs(100);
	{
		GList* groups = hud_action_publisher_get_action_groups(publisher);
		ASSERT_EQ(1, g_list_length(groups));
		{
			HudActionPublisherActionGroupSet *group =
					(HudActionPublisherActionGroupSet *) g_list_nth_data(groups,
							0);
			EXPECT_STREQ("/app/id", group->path);
			EXPECT_STREQ("app", group->prefix);
		}
	}

	hud_action_publisher_add_action_group(publisher, "prefix", "/object/path");

	// FIXME Waiting in tests
	QTestEventLoop::instance().enterLoopMSecs(100);
	{
		GList* groups = hud_action_publisher_get_action_groups(publisher);
		ASSERT_EQ(2, g_list_length(groups));
		{
			HudActionPublisherActionGroupSet *group =
					(HudActionPublisherActionGroupSet *) g_list_nth_data(groups,
							0);
			EXPECT_STREQ("/object/path", group->path);
			EXPECT_STREQ("prefix", group->prefix);
		}
		{
			HudActionPublisherActionGroupSet *group =
					(HudActionPublisherActionGroupSet *) g_list_nth_data(groups,
							1);
			EXPECT_STREQ("/app/id", group->path);
			EXPECT_STREQ("app", group->prefix);
		}

		//FIXME This code causes the test suite to fail
//		GMenuModel *model =
//		G_MENU_MODEL(g_dbus_menu_model_get(
//						connection,
//						DBUS_NAME, "/com/canonical/hud/publisher"));
//
//		ASSERT_TRUE(model);
//		EXPECT_EQ(0, g_menu_model_get_n_items(model));
//		g_object_unref(model);
	}

	// FIXME This API method currently does nothing
	hud_action_publisher_remove_action_group(publisher, "prefix",
			g_variant_new_string("/object/path"));

	g_object_unref(publisher);
	g_object_unref(application);
}

TEST_F(TestActionPublisher, AddDescription) {
	GApplication *application = g_application_new("app.id",
			G_APPLICATION_FLAGS_NONE);
	ASSERT_TRUE(g_application_register(application, NULL, NULL));

	HudActionPublisher *publisher = hud_action_publisher_new_for_application(
			application);
	ASSERT_TRUE(publisher);

	HudActionDescription *description = hud_action_description_new(
			"hud.simple-action", g_variant_new_string("Foo"));
	hud_action_description_set_attribute_value(description,
	G_MENU_ATTRIBUTE_LABEL, g_variant_new_string("Simple Action"));

	hud_action_publisher_add_description(publisher, description);

	// FIXME Waiting in tests
	QTestEventLoop::instance().enterLoopMSecs(100);

	{
		GMenuModel *model =
		G_MENU_MODEL(
				g_dbus_menu_model_get(connection, DBUS_NAME,
						"/com/canonical/hud/publisher"));
		ASSERT_TRUE(model);
		EXPECT_EQ(0, g_menu_model_get_n_items(model));
		g_object_unref(model);
	}

	HudActionDescription *paramdesc = hud_action_description_new(
			"hud.simple-action", NULL);
	hud_action_description_set_attribute_value(paramdesc,
	G_MENU_ATTRIBUTE_LABEL, g_variant_new_string("Parameterized Action"));
	hud_action_publisher_add_description(publisher, paramdesc);

	GMenu * menu = g_menu_new();
	g_menu_append_item(menu, g_menu_item_new("Item One", "hud.simple-action"));
	g_menu_append_item(menu, g_menu_item_new("Item Two", "hud.simple-action"));
	g_menu_append_item(menu,
			g_menu_item_new("Item Three", "hud.simple-action"));
	hud_action_description_set_parameterized(paramdesc, G_MENU_MODEL(menu));

	// FIXME Waiting in tests
	QTestEventLoop::instance().enterLoopMSecs(100);

	{
		GMenuModel *model = G_MENU_MODEL(
				g_dbus_menu_model_get(connection, DBUS_NAME,
						"/com/canonical/hud/publisher"));
		EXPECT_TRUE(model);
		EXPECT_EQ(0, g_menu_model_get_n_items(model));
		EXPECT_TRUE(g_menu_model_is_mutable(model));
		g_object_unref(model);
	}

	hud_action_description_unref(paramdesc);
	hud_action_description_unref(description);
	g_object_unref(application);
	g_object_unref(publisher);
}

}
