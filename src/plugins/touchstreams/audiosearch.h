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
#include <interfaces/core/icoreproxy.h>
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

	class AudioSearch : public QObject
					  , public Media::IPendingAudioSearch
	{
		Q_OBJECT
		Q_INTERFACES (Media::IPendingAudioSearch)

		ICoreProxy_ptr Proxy_;
		Util::QueueManager *Queue_;

		AuthManager *AuthMgr_;
		const QString Query_;

		QList<Media::IPendingAudioSearch::Result> Result_;
	public:
		AudioSearch (ICoreProxy_ptr, const QString&, AuthManager*, Util::QueueManager*, QObject* = 0);

		QObject* GetObject ();
		QList<Result> GetResults () const;
	private slots:
		void handleGotAuthKey (const QString&);
		void handleGotReply ();
		void handleError ();
	signals:
		void ready ();
	};
}
}
