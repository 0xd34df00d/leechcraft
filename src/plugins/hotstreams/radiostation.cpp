/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiostation.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QtDebug>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace HotStreams
{
	RadioStation::RadioStation (const QUrl& url, const QString& name, QNetworkAccessManager *nam, const QString& format)
	: StreamUrl_ (url)
	, Name_ (name)
	, PlaylistFormat_ (format)
	{
		connect (nam->get (QNetworkRequest (url)),
				SIGNAL (finished ()),
				this,
				SLOT (handlePlaylistFetched ()));
	}

	QObject* RadioStation::GetQObject ()
	{
		return this;
	}

	QString RadioStation::GetRadioName () const
	{
		return Name_;
	}

	void RadioStation::RequestNewStream ()
	{
	}

	void RadioStation::handlePlaylistFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		QTemporaryFile file;
		file.setAutoRemove (false);
		if (!file.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open temporary file";
			return;
		}

		file.write (reply->readAll ());
		file.close ();
		emit gotPlaylist (file.fileName (), PlaylistFormat_);
	}
}
}
