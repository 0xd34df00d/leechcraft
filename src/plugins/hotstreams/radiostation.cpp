/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "radiostation.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QtDebug>
#include <interfaces/media/audiostructs.h>

namespace LeechCraft
{
namespace HotStreams
{
	RadioStation::RadioStation (const QUrl& url, const QString& name, QNetworkAccessManager *nam)
	: StreamUrl_ (url)
	, Name_ (name)
	{
		connect (nam->get (QNetworkRequest (url)),
				SIGNAL (finished ()),
				this,
				SLOT (handlePlaylistFetched ()));
	}

	QObject* RadioStation::GetObject ()
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
		emit gotPlaylist (file.fileName (), "pls");
	}
}
}
