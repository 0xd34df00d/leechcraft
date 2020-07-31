/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selfavatarfetcher.h"
#include <QTimer>
#include <QStringList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/threads/futures.h>
#include "avatarstimestampstorage.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	SelfAvatarFetcher::Urls::Urls (const QString& full)
	{
		auto split = full.split ('@', Qt::SkipEmptyParts);
		if (split.size () != 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid full address"
					<< full;
			return;
		}

		auto& name = split [0];
		auto& domain = split [1];
		if (domain.endsWith (".ru"))
			domain.chop (3);

		const auto& base = "http://obraz.foto.mail.ru/" + domain + "/" + name + "/_mrimavatar";

		SmallUrl_ = base + "small";
		BigUrl_ = base + "big";
	}

	SelfAvatarFetcher::SelfAvatarFetcher (QNetworkAccessManager *nam,
			const QString& full, QObject *parent)
	: QObject { parent }
	, NAM_ { nam }
	, Timer_ { new QTimer { this } }
	, FullAddress_ { full }
	, Urls_ { full }
	, PreviousDateTime_ { AvatarsTimestampStorage {}.GetTimestamp (full).value_or (QDateTime {}) }
	{
		if (!IsValid ())
			return;

		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (refetch ()));
		Timer_->setInterval (120 * 60 * 1000);
		Timer_->start ();

		QTimer::singleShot (2000,
				this,
				SLOT (refetch ()));
	}

	bool SelfAvatarFetcher::IsValid () const
	{
		return Urls_.SmallUrl_.isValid ();
	}

	QFuture<QImage> SelfAvatarFetcher::FetchAvatar (IHaveAvatars::Size size) const
	{
		QUrl url;
		switch (size)
		{
		case IHaveAvatars::Size::Thumbnail:
			url = Urls_.SmallUrl_;
			break;
		case IHaveAvatars::Size::Full:
			url = Urls_.BigUrl_;
			break;
		}

		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL for"
					<< static_cast<int> (size)
					<< FullAddress_;
			return Util::MakeReadyFuture<QImage> ({});
		}

		QFutureInterface<QImage> iface;
		iface.reportStarted ();

		const auto reply = NAM_->get (QNetworkRequest { url });
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[iface, reply] () mutable
			{
				reply->deleteLater ();

				const auto& image = QImage::fromData (reply->readAll ());
				iface.reportFinished (&image);
			},
			reply,
			SIGNAL (finished ()),
			reply
		};

		return iface.future ();
	}

	void SelfAvatarFetcher::refetch ()
	{
		const auto reply = NAM_->head (QNetworkRequest (Urls_.SmallUrl_));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleHeadFinished ()));
	}

	void SelfAvatarFetcher::handleHeadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		if (reply->error () == QNetworkReply::ContentNotFoundError)
		{
			qDebug () << Q_FUNC_INFO
					<< "avatar not found for"
					<< FullAddress_;
			return;
		}

		const auto& dt = reply->header (QNetworkRequest::LastModifiedHeader).toDateTime ();
		if (dt <= PreviousDateTime_)
			return;

		PreviousDateTime_ = dt;

		AvatarsTimestampStorage {}.SetTimestamp (FullAddress_, dt);

		emit avatarChanged ();
	}
}
}
}
