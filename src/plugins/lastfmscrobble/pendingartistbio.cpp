/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingartistbio.h"
#include <algorithm>
#include <QNetworkReply>
#include <QDomDocument>
#include <QtDebug>
#include <util/network/handlenetworkreply.h>
#include "util.h"
#include "imagesfetcher.h"

namespace LC
{
namespace Lastfmscrobble
{
	PendingArtistBio::PendingArtistBio (QString name,
			QNetworkAccessManager *nam, bool addImages, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, AddImages_ (addImages)
	{
		Promise_.reportStarted ();

		QMap<QString, QString> params
		{
			{ "artist", name },
			{ "autocorrect", "1" }
		};
		AddLanguageParam (params);
		Util::HandleReplySeq<Util::ErrorInfo<QString>> (Request ("artist.getInfo", nam, params), this) >>
				Util::Visitor
				{
					[this] (const QString& err)
					{
						Util::ReportFutureResult (Promise_, err);
						deleteLater ();
					},
					[this] (const QByteArray& data) { HandleFinished (data); }
				};
	}

	QFuture<Media::IArtistBioFetcher::Result_t> PendingArtistBio::GetFuture ()
	{
		return Promise_.future ();
	}

	void PendingArtistBio::HandleFinished (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			Util::ReportFutureResult (Promise_, "unable to parse reply");
			deleteLater ();
			return;
		}

		Media::ArtistBio bio;
		bio.BasicInfo_ = GetArtistInfo (doc.documentElement ().firstChildElement ("artist"));
		std::reverse (bio.BasicInfo_.Tags_.begin (), bio.BasicInfo_.Tags_.end ());

		if (!AddImages_)
		{
			Util::ReportFutureResult (Promise_, bio);
			deleteLater ();
			return;
		}

		const auto imgFetcher = new ImagesFetcher { bio.BasicInfo_.Name_, NAM_, this };
		connect (imgFetcher,
				&ImagesFetcher::gotImages,
				this,
				[this, bio] (const QList<Media::ArtistImage>& images) mutable
				{
					bio.OtherImages_ = images;
					Util::ReportFutureResult (Promise_, bio);
					deleteLater ();
				});
	}
}
}
