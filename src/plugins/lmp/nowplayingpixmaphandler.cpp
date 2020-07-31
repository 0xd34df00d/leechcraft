/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nowplayingpixmaphandler.h"
#include <QUrl>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "mediainfo.h"

namespace LC
{
namespace LMP
{
	NowPlayingPixmapHandler::NowPlayingPixmapHandler (QObject *parent)
	: QObject (parent)
	, IsValidPixmap_ (false)
	{
	}

	void NowPlayingPixmapHandler::AddSetter (const PixmapSetter_f setter)
	{
		Setters_ << setter;
	}

	void NowPlayingPixmapHandler::HandleSongChanged (const MediaInfo& info,
			const QString& coverPath, const QPixmap& px, bool correct)
	{
		if (coverPath == LastCoverPath_ && correct)
			return;

		if (LastArtist_ == info.Artist_ && IsValidPixmap_ && !correct)
			return;

		LastArtist_ = info.Artist_;
		for (const auto& setter : Setters_)
			setter (px, coverPath);

		IsValidPixmap_ = correct;
		LastCoverPath_ = coverPath;
	}

	QString NowPlayingPixmapHandler::GetLastCoverPath () const
	{
		return LastCoverPath_;
	}

	void NowPlayingPixmapHandler::handleGotArtistImage (const QString& name, const QUrl& url)
	{
		if (name != LastArtist_ || !url.isValid ())
			return;

		if (IsValidPixmap_)
			return;

		auto nam = Core::Instance ().GetProxy ()->GetNetworkAccessManager ();
		connect (nam->get (QNetworkRequest (url)),
				SIGNAL (finished ()),
				this,
				SLOT (handleDownloadedImage ()));
	}

	void NowPlayingPixmapHandler::handleDownloadedImage ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		const auto& pixmap = QPixmap::fromImage (QImage::fromData (reply->readAll ()));
		if (pixmap.isNull ())
			return;

		for (const auto& setter : Setters_)
			setter (pixmap, {});

		LastCoverPath_.clear ();
		IsValidPixmap_ = true;
	}
}
}
