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
		QSharedPointer<ComCanonicalUnityVoiceInterface> voice_interface) :
		m_voice_interface(voice_interface) {
	if (voice_interface == nullptr) {
		throw std::logic_error("Unable to initialize voice");
	}

	LibUnityVoice::UnityVoice::registerMetaTypes();

	connect(m_voice_interface.data(), SIGNAL( HeardSomething() ), this,
			SIGNAL( HeardSomething() ));
	connect(m_voice_interface.data(), SIGNAL( Listening() ), this,
			SIGNAL( Listening() ));
	connect(m_voice_interface.data(), SIGNAL( Loading() ), this,
			SIGNAL( Loading() ));
}

VoiceImpl::~VoiceImpl() {
}

QString VoiceImpl::Listen(const QList<QStringList>& commands) {
	if (commands.size() == 0) {
		return QString();
	}

	QDBusPendingReply<QString> query(m_voice_interface->listen(commands));

	if (query.isError()) {
		qWarning() << query.error();
		return QString();
	}

	return query;
}
