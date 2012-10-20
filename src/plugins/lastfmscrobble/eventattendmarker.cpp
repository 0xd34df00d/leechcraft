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

#include "eventattendmarker.h"
#include <QMap>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QtDebug>
#include "authenticator.h"
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	EventAttendMarker::EventAttendMarker (Authenticator *auth, QNetworkAccessManager *nam, qint64 id, Media::EventAttendType type, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, ID_ (id)
	, Code_ (0)
	{
		switch (type)
		{
		case Media::EventAttendType::None:
			Code_ = 2;
			break;
		case Media::EventAttendType::Maybe:
			Code_ = 1;
			break;
		case Media::EventAttendType::Surely:
			Code_ = 0;
			break;
		}

		if (auth->IsAuthenticated ())
			mark ();
		else
			connect (auth,
					SIGNAL (authenticated ()),
					this,
					SLOT (mark ()));
	}

	void EventAttendMarker::mark ()
	{
		QMap<QString, QString> params;
		params ["event"] = QString::number (ID_);
		params ["status"] = QString::number (Code_);
		auto reply = Request ("event.attend", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void EventAttendMarker::handleFinished ()
	{
		emit finished ();
		deleteLater ();
	}

	void EventAttendMarker::handleError ()
	{
		qWarning () << Q_FUNC_INFO
				<< "error marking event";
		deleteLater ();
	}
}
}
