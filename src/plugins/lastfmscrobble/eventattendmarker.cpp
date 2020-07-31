/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eventattendmarker.h"
#include <QMap>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QtDebug>
#include "authenticator.h"
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	EventAttendMarker::EventAttendMarker (Authenticator *auth, QNetworkAccessManager *nam, qint64 id, Media::EventAttendType type, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, ID_ (id)
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
		sender ()->deleteLater ();
		emit finished ();
		deleteLater ();
	}

	void EventAttendMarker::handleError ()
	{
		sender ()->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< "error marking event";
		deleteLater ();
	}
}
}
