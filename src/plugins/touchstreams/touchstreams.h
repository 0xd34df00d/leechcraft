/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/iaudiopile.h>

namespace LeechCraft
{
namespace Util
{
	class QueueManager;
}

namespace TouchStreams
{
	class AuthManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public Media::IAudioPile
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings Media::IAudioPile)

		ICoreProxy_ptr Proxy_;
		Util::QueueManager *Queue_;

		Util::XmlSettingsDialog_ptr XSD_;
		AuthManager *AuthMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		Media::IPendingAudioSearch* Search (const Media::AudioSearchRequest&);
	private slots:
		void handlePushButton (const QString&);
	};
}
}
