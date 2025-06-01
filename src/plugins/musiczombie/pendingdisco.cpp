/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingdisco.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QFuture>
#include <QPointer>
#include <QtDebug>
#include <util/sll/queuemanager.h>
#include <util/util.h>
#include <util/sll/util.h>
#include <util/sll/prelude.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/urloperator.h>
#include <util/sll/visitor.h>
#include <util/sll/parsejson.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <util/network/handlenetworkreply.h>
#include "util.h"

namespace LC
{
namespace MusicZombie
{
	namespace
	{
		QString NormalizeName (QString name)
		{
			return name.remove ('!');
		}

		QString NormalizeRelease (QString title)
		{
			static QRegularExpression normalizer { "\\([^)]*\\)" };
			return title
					.remove (normalizer)
					.remove (' ')
					.remove ('.')
					.remove (':')
					.toLower ();
		}
	}

	PendingDisco::PendingDisco (Util::QueueManager *queue, const QString& artist, const QString& release,
			const QStringList& hints, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, Artist_ (artist)
	, ReleaseName_ (release.toLower ())
	, Hints_ (Util::MapAs<QSet> (hints, &NormalizeRelease))
	, Queue_ (queue)
	, NAM_ (nam)
	{
		Promise_.reportStarted ();
		RequestArtist (true);
	}

	QFuture<Media::IDiscographyProvider::Result_t> PendingDisco::GetFuture ()
	{
		return Promise_.future ();
	}

	void PendingDisco::RequestArtist (bool strictMatch)
	{
		Queue_->Schedule ([this, strictMatch]
				{
					const auto& artistQuery = strictMatch ?
							("artist:\"" + NormalizeName (Artist_) + "\"") :
							("artist:" + NormalizeName (Artist_));

					QUrl url { "https://musicbrainz.org/ws/2/release/" };
					Util::UrlOperator { url }
							("status", "official")
							("fmt", "json")
							("query", artistQuery);

					const auto& req = SetupRequest (QNetworkRequest { url });
					Util::HandleReplySeq (NAM_->get (req), this) >>
							Util::Visitor
							{
								[this, strictMatch] (const QByteArray& data) { HandleData (data, strictMatch); },
								[this] (const auto&)
								{
									Util::ReportFutureResult (Promise_,
											{ Util::AsLeft, tr ("Error getting candidate releases list.") });
									deleteLater ();
								}
							};
				},
				this);
	}

	void PendingDisco::HandleData (const QByteArray& data, bool wasStrict)
	{
		const auto& releases = Util::ParseJson (data, Q_FUNC_INFO).toMap () ["releases"].toList ();

		Artist2Releases_t artist2releases;
		for (const auto& releaseVar : releases)
		{
			const auto& release = releaseVar.toMap ();

			const auto& title = NormalizeRelease (release ["title"].toString ());
			for (const auto& artistVar : release ["artist-credit"].toList ())
			{
				const auto& id = artistVar.toMap () ["artist"].toMap () ["id"].toString ();
				artist2releases [id] << title;
			}
		}

		if (artist2releases.isEmpty ())
		{
			if (wasStrict)
				RequestArtist (false);
			else
			{
				Util::ReportFutureResult (Promise_, { Util::AsLeft, tr ("No artists were found.") });
				deleteLater ();
			}

			return;
		}

		if (Hints_.isEmpty ())
			HandleDataNoHints (artist2releases);
		else
			HandleDataWithHints (artist2releases);

	}

	void PendingDisco::HandleDataNoHints (const Artist2Releases_t& artist2releases)
	{
		const auto maxElem = std::max_element (artist2releases.begin (), artist2releases.end (),
				Util::ComparingBy (&QSet<QString>::size));
		qDebug () << Q_FUNC_INFO
				<< "max size:"
				<< *maxElem;

		HandleGotID (maxElem.key ());
	}

	void PendingDisco::HandleDataWithHints (Artist2Releases_t& artist2releases)
	{
		const auto& xSizes = Util::Map (Util::Stlize (artist2releases),
				[this] (auto&& pair)
				{
					pair.second.intersect (Hints_);
					return QPair { pair.first, pair.second.size () };
				});

		const auto maxElem = std::max_element (xSizes.begin (), xSizes.end (), Util::ComparingBy (Util::Snd));

		qDebug () << Q_FUNC_INFO
				<< "intersections size:"
				<< maxElem->second;

		if (maxElem->second)
			HandleGotID (maxElem->first);
		else
			HandleDataNoHints (artist2releases);
	}

	void PendingDisco::HandleGotID (const QString& id)
	{
		static const QString pref { "http://musicbrainz.org/ws/2/release?limit=100&inc=recordings+release-groups&status=official&artist=" };
		const QUrl url { pref + id };

		Queue_->Schedule ([this, url]
				{
					const auto reply = NAM_->get (SetupRequest (QNetworkRequest { url }));
					Util::HandleReplySeq (reply, this) >>
							Util::Visitor
							{
								[this] (const QByteArray& data) { HandleLookupFinished (data); },
								[this] (const auto&)
								{
									Util::ReportFutureResult (Promise_,
											{ Util::AsLeft, tr ("Error getting artist releases list.") });
									deleteLater ();
								}
							};
				},
				this);
	}

	namespace
	{
		auto ParseMediumList (const QDomElement& mediumList)
		{
			return Util::Map (Util::DomChildren (mediumList, "medium"),
					[] (const auto& medium)
					{
						return Util::Map (Util::DomChildren (medium.firstChildElement ("track-list"), "track"),
								[] (const auto& trackElem)
								{
									const int num = trackElem.firstChildElement ("number").text ().toInt ();

									const auto& recElem = trackElem.firstChildElement ("recording");
									const auto& title = recElem.firstChildElement ("title").text ();
									const int length = recElem.firstChildElement ("length").text ().toInt () / 1000;
									return Media::ReleaseTrackInfo { num, title, length };
								});
					});
		}

		Media::ReleaseInfo::Type GetReleaseType (const QDomElement& releaseElem)
		{
			const auto& elem = releaseElem.firstChildElement ("release-group");
			if (elem.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< "null element";
				return Media::ReleaseInfo::Type::Other;
			}

			const auto& type = elem.attribute ("type");

			static const QMap<QString, Media::ReleaseInfo::Type> map
			{
				{ "Album", Media::ReleaseInfo::Type::Standard },
				{ "EP", Media::ReleaseInfo::Type::EP },
				{ "Single", Media::ReleaseInfo::Type::Single },
				{ "Compilation", Media::ReleaseInfo::Type::Compilation },
				{ "Live", Media::ReleaseInfo::Type::Live },
				{ "Soundtrack", Media::ReleaseInfo::Type::Soundtrack },
				{ "Other", Media::ReleaseInfo::Type::Other }
			};

			if (map.contains (type))
				return map.value (type);

			qWarning () << Q_FUNC_INFO
					<< "unknown type:"
					<< type;
			return Media::ReleaseInfo::Type::Other;
		}
	}

	void PendingDisco::HandleLookupFinished (const QByteArray& data)
	{
		deleteLater ();

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< data;
			Util::ReportFutureResult (Promise_, { Util::AsLeft, tr ("Unable to parse MusicBrainz reply.") });
			return;
		}

		QMap<QString, QMap<QString, Media::ReleaseInfo>> infos;

		const auto& releaseList = doc.documentElement ().firstChildElement ("release-list");
		for (const auto& releaseElem : Util::DomChildren (releaseList, "release"))
		{
			auto elemText = [&releaseElem] (const QString& sub)
			{
				return releaseElem.firstChildElement (sub).text ();
			};

			if (elemText ("status") != "Official")
				continue;

			const auto& dateStr = elemText ("date");
			const int dashPos = dateStr.indexOf ('-');
			const int date = (dashPos > 0 ? dateStr.left (dashPos) : dateStr).toInt ();
			if (date < 1000)
				continue;

			const auto& title = elemText ("title");
			if (!ReleaseName_.isEmpty () && title.toLower () != ReleaseName_)
				continue;

			Media::ReleaseInfo info
			{
				releaseElem.attribute ("id"),
				title,
				date,
				GetReleaseType (releaseElem),
				ParseMediumList (releaseElem.firstChildElement ("medium-list"))
			};

			infos [title] [elemText ("country")] = info;
		}

		QList<Media::ReleaseInfo> releases;
		for (const auto& countries : infos)
		{
			Media::ReleaseInfo release;
			if (countries.contains ("US"))
				release = countries ["US"];
			else if (!countries.isEmpty ())
				release = countries.begin ().value ();

			releases << release;
		}

		std::sort (releases.begin (), releases.end (),
				Util::ComparingBy (&Media::ReleaseInfo::Year_));
		Util::ReportFutureResult (Promise_, releases);
	}
}
}
