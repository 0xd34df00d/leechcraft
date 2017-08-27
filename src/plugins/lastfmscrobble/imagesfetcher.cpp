/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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

namespace LeechCraft
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
		const QNetworkRequest req { GetUrl ("artist/photos/pageurl") };
		HandleReply (NAM_->post (req, "artist=" + QUrl::toPercentEncoding (artist)),
				[this] (const QByteArray& data) { HandlePageUrl (data); });
	}

	void ImagesFetcher::HandleDone ()
	{
		emit gotImages (Images_);
		deleteLater ();
	}

	template<typename F>
	void ImagesFetcher::HandleReply (QNetworkReply *reply, F f)
	{
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[reply, f]
			{
				reply->deleteLater ();
				f (reply->readAll ());
			},
			reply,
			SIGNAL (finished ()),
			this
		};
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

		HandleReply (NAM_->get (QNetworkRequest { url }),
				[this] (const QByteArray& data) { HandleImagesPageFetched (data); });
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
				"form-data; name=\"contents\"; filename=\"contents\"");
		contentsPart.setBody (data);

		auto multipart = new QHttpMultiPart { QHttpMultiPart::FormDataType };
		multipart->append (contentsPart);

		const auto reply = NAM_->post (QNetworkRequest { GetUrl ("artist/photos/parsepage") }, multipart);
		multipart->setParent (reply);

		HandleReply (reply,
				[this] (const QByteArray& data) { HandlePageParsed (data); });
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
