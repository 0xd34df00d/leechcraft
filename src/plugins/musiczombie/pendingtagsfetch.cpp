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

#include "pendingtagsfetch.h"
#include <functional>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <util/queuemanager.h>
#include "chroma.h"

namespace LeechCraft
{
namespace MusicZombie
{
	const QString ApiKey = "rBE9CXpr";

	PendingTagsFetch::PendingTagsFetch (Util::QueueManager *queue,
			QNetworkAccessManager *nam, const QString& filename)
	: Queue_ (queue)
	, NAM_ (nam)
	, Filename_ (filename)
	{
		auto worker = [filename] ()
		{
			Chroma chroma;
			try
			{
				return chroma (filename);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				return Chroma::Result ();
			}
		};

		auto watcher = new QFutureWatcher<Chroma::Result> (this);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotFingerprint ()));
		auto future = QtConcurrent::run (std::function<Chroma::Result ()> (worker));
		watcher->setFuture (future);
	}

	QObject* PendingTagsFetch::GetObject ()
	{
		return this;
	}

	Media::AudioInfo PendingTagsFetch::GetResult () const
	{
		return Info_;
	}

	void PendingTagsFetch::Request (const QByteArray& fp, int duration)
	{
		QUrl url ("http://api.acoustid.org/v2/lookup");
		url.addQueryItem ("client", ApiKey);
		url.addQueryItem ("duration", QString::number (duration));
		url.addQueryItem ("fingerprint", QString::fromLatin1 (fp));
		//url.addQueryItem ("meta", "recordings+releasegroups+compress");

		auto reply = NAM_->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void PendingTagsFetch::handleGotFingerprint ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<Chroma::Result>*> (sender ());
		watcher->deleteLater ();

		const auto& result = watcher->result ();
		const auto& fp = result.FP_;
		if (fp.isEmpty ())
		{
			emit ready (Filename_, Media::AudioInfo ());
			deleteLater ();
		}

		Queue_->Schedule ([this, result] () { Request (result.FP_, result.Duration_); }, this);
	}

	void PendingTagsFetch::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		deleteLater ();
	}
}
}
