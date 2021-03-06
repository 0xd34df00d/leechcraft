/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingtagsfetch.h"
#include <QtConcurrentRun>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/threads/futures.h>
#include "chroma.h"

namespace LC
{
namespace MusicZombie
{
	const QString ApiKey = "rBE9CXpr";

	PendingTagsFetch::PendingTagsFetch (Util::QueueManager *queue,
			QNetworkAccessManager *nam, const QString& filename)
	: Queue_ (queue)
	, NAM_ (nam)
	{
		auto worker = [filename]
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

		Util::Sequence (this, QtConcurrent::run (worker)) >>
				[this] (const Chroma::Result& result)
				{
					const auto& fp = result.FP_;
					if (fp.isEmpty ())
					{
						Util::ReportFutureResult (Promise_, Media::AudioInfo {});
						deleteLater ();
						return;
					}

					Queue_->Schedule ([this, result] { Request (result.FP_, result.Duration_); }, this);
				};
	}

	QFuture<Media::AudioInfo> PendingTagsFetch::GetFuture ()
	{
		return Promise_.future ();
	}

	void PendingTagsFetch::Request (const QByteArray& fp, int duration)
	{
		QUrl url ("http://api.acoustid.org/v2/lookup");
		Util::UrlOperator { url }
				("client", ApiKey)
				("duration", QString::number (duration))
				("fingerprint", QString::fromLatin1 (fp))
				("meta", "recordings+releasegroups+releases+tracks+sources+usermeta")
				("format", "xml");

		auto reply = NAM_->get (QNetworkRequest (url));
		connect (reply,
				&QNetworkReply::finished,
				this,
				&PendingTagsFetch::HandleReplyFinished);
	}

	void PendingTagsFetch::HandleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing"
					<< data;
			Util::ReportFutureResult (Promise_, Media::AudioInfo {});
			return;
		}

		const auto& result = doc.documentElement ()
				.firstChildElement ("results")
				.firstChildElement ("result");
		if (result.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no results";
			Util::ReportFutureResult (Promise_, Media::AudioInfo {});
			return;
		}

		Media::AudioInfo info;

		const auto& recordingElem = result
				.firstChildElement ("recordings")
				.firstChildElement ("recording");

		info.Title_ = recordingElem.firstChildElement ("title").text ();

		const auto& releaseGroupElem = recordingElem
				.firstChildElement ("releasegroups")
				.firstChildElement ("releasegroup");
		info.Album_ = releaseGroupElem
				.firstChildElement ("title").text ();

		const auto& releaseElem = releaseGroupElem
				.firstChildElement ("releases")
				.firstChildElement ("release");

		const auto& dateElem = releaseElem.firstChildElement ("date");
		info.Year_ = dateElem.firstChildElement ("year").text ().toInt ();

		auto trackElem = releaseElem
				.firstChildElement ("mediums")
				.firstChildElement ("medium")
				.firstChildElement ("tracks")
				.firstChildElement ("track");
		while (!trackElem.isNull ())
		{
			info.TrackNumber_ = trackElem.firstChildElement ("position").text ().toInt ();
			if (trackElem.firstChildElement ("title").text () == info.Title_)
				break;

			trackElem = trackElem.nextSiblingElement ("track");
		}

		QStringList artists;
		auto artistElem = recordingElem
				.firstChildElement ("artists")
				.firstChildElement ("artist");
		while (!artistElem.isNull ())
		{
			artists << artistElem.firstChildElement ("name").text ();
			artistElem = artistElem.nextSiblingElement ("artist");
		}
		info.Artist_ = artists.join (" feat ");

		Util::ReportFutureResult (Promise_, info);
	}
}
}
