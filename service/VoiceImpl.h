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

#ifndef HUD_SERVICE_VOICEIMPL_H_
#define HUD_SERVICE_VOICEIMPL_H_

#include <libunityvoice/VoiceInterface.h>
#include <service/Voice.h>
#include <service/ItemStore.h>

#include <memory>

namespace hud
{
namespace service
{

class VoiceImpl : public Voice
{
Q_OBJECT

public:
  explicit VoiceImpl( const QString &service, const QString &path,
      const QDBusConnection &connection, QObject *parent = 0 );
  virtual ~VoiceImpl();

  QString Listen( QSharedPointer< ItemStore > items ) override;

private:
  ComCanonicalUnityVoiceInterface voice_interface;
};

} // namespace service
} // namespace hud

#endif // HUD_SERVICE_VOICEIMPL_H_
