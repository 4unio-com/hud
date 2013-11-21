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
 * Author: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <service/VoiceImpl.h>

#include <common/DBusTypes.h>
#include <libqtdbusmock/DBusMock.h>

#include <QSignalSpy>
#include <gtest/gtest.h>

using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace hud::common;
using namespace hud::service;

namespace {

class TestVoice: public Test {
protected:
	TestVoice() :
			mock(dbus) {
		mock.registerCustomMock(DBusTypes::UNITY_VOICE_DBUS_NAME,
				DBusTypes::UNITY_VOICE_DBUS_PATH,
				ComCanonicalUnityVoiceInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);

		dbus.startServices();

		voice_interface.reset(
				new ComCanonicalUnityVoiceInterface(
						DBusTypes::UNITY_VOICE_DBUS_NAME,
						DBusTypes::UNITY_VOICE_DBUS_PATH,
						dbus.sessionConnection()));
	}

	virtual ~TestVoice() {
	}

	virtual OrgFreedesktopDBusMockInterface & unityVoiceMock() {
		return mock.mockInterface(DBusTypes::UNITY_VOICE_DBUS_NAME,
				DBusTypes::UNITY_VOICE_DBUS_PATH,
				ComCanonicalUnityVoiceInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);
	}

	DBusTestRunner dbus;
	DBusMock mock;
	QSharedPointer<ComCanonicalUnityVoiceInterface> voice_interface;
};

TEST_F( TestVoice, NullInterface ) {
	// construct VoiceImpl with an invalid pointer

	auto null_interface = QSharedPointer<ComCanonicalUnityVoiceInterface>(
			nullptr);

	EXPECT_THROW(VoiceImpl voice( null_interface ), std::logic_error);
}

TEST_F( TestVoice, Signals ) {
	// check that signals are patched through VoiceImpl

	VoiceImpl voice(voice_interface);

	QSignalSpy m_heard_something_spy(&voice, SIGNAL( HeardSomething() ));
	QSignalSpy m_listening_spy(&voice, SIGNAL( Listening() ));
	QSignalSpy m_loading_spy(&voice, SIGNAL( Loading() ));

	EXPECT_TRUE(m_heard_something_spy.empty());
	emit voice_interface->HeardSomething();
	EXPECT_FALSE(m_heard_something_spy.empty());

	EXPECT_TRUE(m_listening_spy.empty());
	emit voice_interface->Listening();
	EXPECT_FALSE(m_listening_spy.empty());

	EXPECT_TRUE(m_loading_spy.empty());
	emit voice_interface->Loading();
	EXPECT_FALSE(m_loading_spy.empty());
}

TEST_F( TestVoice, ListenNoCommands ) {
	// call listen() with an empty commands list

	VoiceImpl voice(voice_interface);
	QList<QStringList> commands;

	unityVoiceMock().AddMethod(DBusTypes::UNITY_VOICE_DBUS_NAME, "listen",
			"aas", "s", "ret = 'new file'").waitForFinished();

	EXPECT_NE("new file", voice.listen(commands));

	unityVoiceMock().AddMethod(DBusTypes::UNITY_VOICE_DBUS_NAME, "listen",
			"aas", "s", "ret = 'open file'").waitForFinished();

	EXPECT_NE("open file", voice.listen(commands));
}

TEST_F( TestVoice, Listen ) {
	// call listen() with a valid commands list

	VoiceImpl voice(voice_interface);

	QList<QStringList> commands;
	commands.append( { "new", "file" });
	commands.append( { "open", "file" });

	unityVoiceMock().AddMethod(DBusTypes::UNITY_VOICE_DBUS_NAME, "listen",
			"aas", "s", "ret = 'new file'").waitForFinished();

	EXPECT_EQ("new file", voice.listen(commands));

	unityVoiceMock().AddMethod(DBusTypes::UNITY_VOICE_DBUS_NAME, "listen",
			"aas", "s", "ret = 'open file'").waitForFinished();

	EXPECT_EQ("open file", voice.listen(commands));
}

} // namespace
