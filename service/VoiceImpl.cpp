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

using namespace hud::service;

VoiceImpl::VoiceImpl(
		QSharedPointer<ComCanonicalUnityVoiceInterface> voiceInterface) :
		m_voiceInterface(voiceInterface) {
	connect(m_voiceInterface.data(), SIGNAL( HeardSomething() ), this,
			SIGNAL( HeardSomething() ));
	connect(m_voiceInterface.data(), SIGNAL( Listening() ), this,
			SIGNAL( Listening() ));
	connect(m_voiceInterface.data(), SIGNAL( Loading() ), this,
			SIGNAL( Loading() ));
}

VoiceImpl::~VoiceImpl() {
}

QString VoiceImpl::listen(const QList<QStringList>& commands) {
	// return immediately if no commands where supplied
	if (commands.isEmpty()) {
		return QString();
	}

	// call voice listen() asynchronously
	QDBusPendingCall listen_async = m_voiceInterface->asyncCall("listen",
			QVariant::fromValue(commands));

	// connect a call waiter to the async call
	QDBusPendingCallWatcher watcher(listen_async, this);
	connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
			SLOT(listenFinished(QDBusPendingCallWatcher*)));

	// wait for async call to complete
	m_listen_wait.exec();

	// return query set by listenFinished
	return m_query;
}

void VoiceImpl::listenFinished(QDBusPendingCallWatcher *call) {
	QDBusPendingReply<QString> query = *call;

	// set m_query accordingly
	if (query.isError()) {
		qWarning() << query.error();
		m_query = "";
	} else {
		m_query = query;
	}

	// notify listen() that the async call is complete
	m_listen_wait.quit();
}
