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

namespace LeechCraft
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
		Q_FOREACH (const auto& setter, Setters_)
			setter (px, coverPath);

		IsValidPixmap_ = correct;
		LastCoverPath_ = coverPath;
	}
}
}
