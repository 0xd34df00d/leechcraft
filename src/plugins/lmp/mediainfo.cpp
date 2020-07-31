/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mediainfo.h"
#include <QDataStream>

namespace LC
{
namespace LMP
{
	MediaInfo::MediaInfo (const QString& localPath,
			const QString& artist, const QString& album, const QString& title,
			const QStringList& genres,
			qint32 length, qint32 year, qint32 trackNumber)
	: LocalPath_ { localPath }
	, Artist_ { artist }
	, Album_ { album }
	, Title_ { title }
	, Genres_ { genres }
	, Length_ { length }
	, Year_ { year }
	, TrackNumber_ { trackNumber }
	{
	}

	MediaInfo& MediaInfo::operator= (const Media::AudioInfo& info)
	{
		Artist_ = info.Artist_;
		Album_ = info.Album_;
		Title_ = info.Title_;
		Genres_ = info.Genres_;
		Length_ = info.Length_;
		Year_ = info.Year_;
		TrackNumber_ = info.TrackNumber_;
		Additional_ = info.Other_;

		if (Additional_.contains ("URL"))
		{
			const auto& url = Additional_.take ("URL").toUrl ();
			if (url.isLocalFile ())
				LocalPath_ = url.toLocalFile ();
		}

		return *this;
	}

	bool MediaInfo::IsUseless () const
	{
		return (Artist_ + Album_ + Title_).trimmed ().isEmpty ();
	}

	MediaInfo::operator Media::AudioInfo () const
	{
		Media::AudioInfo aInfo =
		{
			Artist_,
			Album_,
			Title_,
			Genres_,
			Length_,
			Year_,
			TrackNumber_,
			Additional_
		};
		aInfo.Other_ ["URL"] = QUrl::fromLocalFile (LocalPath_);
		return aInfo;
	}

	MediaInfo MediaInfo::FromAudioInfo (const Media::AudioInfo& info)
	{
		MediaInfo result;
		result = info;
		return result;
	}

	QDataStream& operator<< (QDataStream& out, const MediaInfo& info)
	{
		out << info.LocalPath_
				<< info.Artist_
				<< info.Album_
				<< info.Title_
				<< info.Genres_
				<< info.Length_
				<< info.Year_
				<< info.TrackNumber_
				<< info.Additional_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, MediaInfo& info)
	{
		in >> info.LocalPath_
				>> info.Artist_
				>> info.Album_
				>> info.Title_
				>> info.Genres_
				>> info.Length_
				>> info.Year_
				>> info.TrackNumber_
				>> info.Additional_;
		return in;
	}
}
}
