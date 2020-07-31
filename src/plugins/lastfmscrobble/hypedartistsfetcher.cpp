/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hypedartistsfetcher.h"
#include <algorithm>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/sll/domchildrenrange.h>
#include <util/network/handlenetworkreply.h>
#include <util/threads/futures.h>
#include "util.h"
#include "pendingartistbio.h"

namespace LC
{
namespace Lastfmscrobble
{
	HypedArtistsFetcher::HypedArtistsFetcher (QNetworkAccessManager *nam, Media::IHypesProvider::HypeType type, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	{
		Promise_.reportStarted ();

		QMap<QString, QString> params;
		params ["limit"] = "20";
		const auto& method = type == Media::IHypesProvider::HypeType::NewArtists ?
				"chart.getHypedArtists" :
				"chart.getTopArtists";
		auto reply = Request (method, nam, params);
		Util::HandleReplySeq (reply, this) >>
				Util::Visitor
				{
					[this] (Util::Void)
					{
						Util::ReportFutureResult (Promise_, QString { "Unable to issue Last.FM API request." });
						deleteLater ();
					},
					[this] (const QByteArray& data) { HandleFinished (data); }
				};
	}

	QFuture<Media::IHypesProvider::HypeQueryResult_t> HypedArtistsFetcher::GetFuture ()
	{
		return Promise_.future ();
	}

	void HypedArtistsFetcher::DecrementWaiting ()
	{
		if (--InfoCount_)
			return;

		Util::ReportFutureResult (Promise_, Infos_);
		deleteLater ();
	}

	void HypedArtistsFetcher::HandleFinished (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			Util::ReportFutureResult (Promise_, QString { "Unable to parse Last.FM response." });
			deleteLater ();
			return;
		}

		for (const auto& artist : Util::DomChildren (doc.documentElement ().firstChildElement ("artists"), "artist"))
		{
			auto getText = [&artist] (const QString& name)
			{
				return artist.firstChildElement (name).text ();
			};

			const auto& name = getText ("name");

			Infos_ << Media::HypedArtistInfo
			{
				Media::ArtistInfo
				{
					name,
					QString (),
					QString (),
					GetImage (artist, "medium"),
					GetImage (artist, "extralarge"),
					getText ("url"),
					Media::TagInfos_t ()
				},
				getText ("percentagechange").toInt (),
				getText ("playcount").toInt (),
				getText ("listeners").toInt ()
			};

			auto pendingBio = new PendingArtistBio (name, NAM_, false, this);
			Util::Sequence (this, pendingBio->GetFuture ()) >>
					Util::Visitor
					{
						[] (const QString&) {},
						[this] (const Media::ArtistBio& info)
						{
							const auto& name = info.BasicInfo_.Name_;
							const auto pos = std::find_if (Infos_.begin (), Infos_.end (),
									[&name] (const auto& other) { return other.Info_.Name_ == name; });
							if (pos != Infos_.end ())
								pos->Info_ = info.BasicInfo_;
						}
					}.Finally ([this] { DecrementWaiting (); });
		}

		InfoCount_ = Infos_.size ();
		if (!InfoCount_)
		{
			Util::ReportFutureResult (Promise_, Infos_);
			deleteLater ();
		}
	}
}
}
