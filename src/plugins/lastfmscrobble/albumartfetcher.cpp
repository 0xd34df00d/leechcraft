/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartfetcher.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QStringList>
#include <QFuture>
#include <util/network/handlenetworkreply.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	AlbumArtFetcher::AlbumArtFetcher (const Media::AlbumInfo& albumInfo, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	{
		Promise_.reportStarted ();

		const QMap<QString, QString> params
		{
			{ "artist", albumInfo.Artist_ },
			{ "album", albumInfo.Album_ },
			{ "autocorrect", "1" }
		};

		const auto reply = Request ("album.getInfo", proxy->GetNetworkAccessManager (), params);
		Util::HandleReplySeq<Util::ErrorInfo<QString>> (reply, this) >>
				Util::Visitor
				{
					[this] (const QString& error) { Util::ReportFutureResult (Promise_, error); },
					[this] (const QByteArray& result) { HandleReplyFinished (result); }
				}.Finally ([this] { deleteLater (); });
	}

	QFuture<Media::IAlbumArtProvider::Result_t> AlbumArtFetcher::GetFuture ()
	{
		return Promise_.future ();
	}

	void AlbumArtFetcher::HandleReplyFinished (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			Util::ReportFutureResult (Promise_, "Unable to parse reply.");
			return;
		}

		const auto& elems = doc.elementsByTagName ("image");

		static const QStringList sizes
		{
			"mega",
			"extralarge",
			"large",
			"medium",
			"small",
			""
		};

		for (const auto& size : sizes)
			for (int i = 0; i < elems.size (); ++i)
			{
				const auto& elem = elems.at (i).toElement ();
				const auto& text = elem.text ();
				if (elem.attribute ("size") == size && !text.isEmpty ())
				{
					Util::ReportFutureResult (Promise_, QList { QUrl { elem.text () } });
					break;
				}
			}
	}
}
}
