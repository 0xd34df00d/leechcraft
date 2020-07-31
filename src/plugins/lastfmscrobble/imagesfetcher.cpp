/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagesfetcher.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/parsejson.h>
#include <util/network/lcserviceoverride.h>
#include <util/network/handlenetworkreply.h>

namespace LC
{
namespace Lastfmscrobble
{
	namespace
	{
		QString GetUrl (const QString& path)
		{
			return Util::GetServiceUrl ({ "leechcraft.org", 12000, "LASTFM_IMAGES" }, path);
		}
	}

	ImagesFetcher::ImagesFetcher (const QString& artist,
			QNetworkAccessManager *nam, QObject *parent)
	: QObject { parent }
	, Artist_ { artist }
	, NAM_ { nam }
	{
		auto reply = NAM_->post (QNetworkRequest { GetUrl ("artist/photos/pageurl") }, "artist=" + QUrl::toPercentEncoding (artist));
		Util::Sequence (this, Util::HandleReply<Util::ErrorInfo<QString>> (reply, this)) >>
				Util::Visitor
				{
					[this] (const QByteArray& data) { HandlePageUrl (data); },
					[this] (const QString& err) { HandleError (err); }
				};
	}

	void ImagesFetcher::HandleError (const QString& err)
	{
		qWarning () << Q_FUNC_INFO << err;
		HandleDone ();
	}

	void ImagesFetcher::HandleDone ()
	{
		emit gotImages (Images_);
		deleteLater ();
	}

	void ImagesFetcher::HandlePageUrl (const QByteArray& data)
	{
		const auto& result = Util::ParseJson (data, Q_FUNC_INFO);
		const auto& url = result.toMap () ["imagesUrl"].toString ();

		if (url.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "got empty page URL for artist"
					<< Artist_;
			HandleDone ();
			return;
		}

		const auto reply = NAM_->get (QNetworkRequest { url });
		Util::Sequence (this, Util::HandleReply<Util::ErrorInfo<QString>> (reply, this)) >>
				Util::Visitor
				{
					[this] (const QByteArray& data) { HandleImagesPageFetched (data); },
					[this] (const QString& err) { HandleError (err); }
				};
	}

	void ImagesFetcher::HandleImagesPageFetched (const QByteArray& data)
	{
		if (data.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no data from last.fm for artist"
					<< Artist_;
			HandleDone ();
			return;
		}

		QHttpPart contentsPart;
		contentsPart.setHeader (QNetworkRequest::ContentDispositionHeader,
				R"(form-data; name="contents"; filename="contents")");
		contentsPart.setBody (data);

		auto multipart = new QHttpMultiPart { QHttpMultiPart::FormDataType };
		multipart->append (contentsPart);

		const auto reply = NAM_->post (QNetworkRequest { GetUrl ("artist/photos/parsepage") }, multipart);
		multipart->setParent (reply);

		Util::Sequence (this, Util::HandleReply<Util::ErrorInfo<QString>> (reply, this)) >>
				Util::Visitor
				{
					[this] (const QByteArray& data) { HandlePageParsed (data); },
					[this] (const QString& err) { HandleError (err); }
				};
	}

	void ImagesFetcher::HandlePageParsed (const QByteArray& data)
	{
		const auto& result = Util::ParseJson (data, Q_FUNC_INFO).toMap ();
		if (result.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse page for artist"
					<< Artist_;
			HandleDone ();
			return;
		}

		for (const auto& dataVar : result ["photoList"].toList ())
		{
			const auto& data = dataVar.toMap ();

			const auto& thumb = data ["thumb"].toString ();
			const auto& full = data ["full"].toString ();

			Images_ << Media::ArtistImage { {}, {}, {}, thumb, full };
		}

		HandleDone ();
	}
}
}
