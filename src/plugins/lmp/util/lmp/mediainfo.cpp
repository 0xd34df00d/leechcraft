/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mediainfo.h"
#include <QDataStream>

namespace LC::LMP
{
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
		MediaInfo mi
		{
			.Artist_ = info.Artist_,
			.Album_ = info.Album_,
			.Title_ = info.Title_,
			.Genres_ = info.Genres_,
			.Length_ = info.Length_,
			.Year_ = info.Year_,
			.TrackNumber_ = info.TrackNumber_,
			.Additional_ = info.Other_,
		};

		if (mi.Additional_.contains ("URL"))
		{
			const auto& url = mi.Additional_.take ("URL").toUrl ();
			if (url.isLocalFile ())
				mi.LocalPath_ = url.toLocalFile ();
		}

		return mi;
	}

	bool operator== (const MediaInfo& l, const MediaInfo& r)
	{
		return l.LocalPath_ == r.LocalPath_ &&
				l.Artist_ == r.Artist_ &&
				l.Album_ == r.Album_ &&
				l.Title_ == r.Title_ &&
				l.Genres_ == r.Genres_ &&
				l.Length_ == r.Length_ &&
				l.Year_ == r.Year_ &&
				l.TrackNumber_ == r.TrackNumber_;
	}

	bool operator!= (const MediaInfo& l, const MediaInfo& r)
	{
		return !(l == r);
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
