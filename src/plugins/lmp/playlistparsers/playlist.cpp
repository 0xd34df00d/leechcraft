/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlist.h"
#include <algorithm>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/util.h>
#include "mediainfo.h"

namespace LC
{
namespace LMP
{
	PlaylistItem::PlaylistItem (const AudioSource& source)
	: Source_ { source }
	{
	}

	PlaylistItem::PlaylistItem (const AudioSource& source, const QVariantMap& additional)
	: Source_ { source }
	, Additional_ { additional }
	{
	}

	namespace
	{
		QVariantMap FromMediaInfo (const MediaInfo& info)
		{
			QVariantMap result;

			for (const auto& pair : Util::Stlize (info.Additional_))
				if (pair.second.canConvert<QString> ())
					result [pair.first] = pair.second;

			const auto pairs =
			{
				QPair<QString, QVariant> { "LMP/HasMediaInfo", true },
				{ "LMP/LocalPath", info.LocalPath_ },
				{ "LMP/Artist", info.Artist_ },
				{ "LMP/Album", info.Album_ },
				{ "LMP/Title", info.Title_ },
				{ "LMP/Genres", info.Genres_.join (" / ") },
				{ "LMP/Length", info.Length_ },
				{ "LMP/Year", info.Year_ },
				{ "LMP/TrackNumber", info.TrackNumber_ }
			};
			for (const auto& pair : pairs)
				result [pair.first] = pair.second;

			return result;
		}
	}

	PlaylistItem::PlaylistItem (const AudioSource& source, const MediaInfo& media)
	: Source_ { source }
	, Additional_ { FromMediaInfo (media) }
	{
	}

	boost::optional<MediaInfo> PlaylistItem::GetMediaInfo () const
	{
		static const QSet<QString> knownFields
		{
			"HasMediaInfo",
			"LocalPath",
			"Artist",
			"Album",
			"Title",
			"Genres",
			"Length",
			"Year",
			"TrackNumber",
		};

		if (!Additional_ ["LMP/HasMediaInfo"].toBool ())
			return {};

		MediaInfo info
		{
			Additional_ ["LMP/LocalPath"].toString (),
			Additional_ ["LMP/Artist"].toString (),
			Additional_ ["LMP/Album"].toString (),
			Additional_ ["LMP/Title"].toString (),
			Additional_ ["LMP/Genres"].toString ().split (" / ", Qt::SkipEmptyParts),
			Additional_ ["LMP/Length"].toInt (),
			Additional_ ["LMP/Year"].toInt (),
			Additional_ ["LMP/TrackNumber"].toInt ()
		};

		for (const auto& pair : Util::Stlize (Additional_))
			if (!knownFields.contains (pair.first))
				info.Additional_ [pair.first] = pair.second;

		return info;
	}

	Playlist::Playlist (const QList<PlaylistItem>& items)
	: Playlist_ { items }
	{
		for (const auto& item : items)
			UrlsSet_ << item.Source_.ToUrl ();
	}

	Playlist::Playlist (const QList<AudioSource>& sources)
	: Playlist { Util::Map (sources, [] (const AudioSource& src) { return PlaylistItem { src }; }) }
	{
	}

	Playlist::const_iterator Playlist::begin () const
	{
		return Playlist_.begin ();
	}

	Playlist::iterator Playlist::begin ()
	{
		return Playlist_.begin ();
	}

	Playlist::const_iterator Playlist::end () const
	{
		return Playlist_.end ();
	}

	Playlist::iterator Playlist::end ()
	{
		return Playlist_.end ();
	}

	Playlist::iterator Playlist::erase (iterator it)
	{
		return Playlist_.erase (it);
	}

	Playlist& Playlist::Append (const PlaylistItem& item)
	{
		if (UrlsSet_.contains (item.Source_.ToUrl ()))
			return *this;

		Playlist_ << item;
		UrlsSet_ << item.Source_.ToUrl ();
		return *this;
	}

	Playlist& Playlist::operator+= (const AudioSource& src)
	{
		Append (PlaylistItem { src });
		return *this;
	}

	Playlist& Playlist::operator+= (const Playlist& playlist)
	{
		for (const auto& item : playlist.Playlist_)
			Append (item);
		return *this;
	}

	QList<AudioSource> Playlist::ToSources () const
	{
		QList<AudioSource> result;
		result.reserve (Playlist_.size ());
		for (const auto& item : Playlist_)
			result << item.Source_;
		return result;
	}

	bool Playlist::IsEmpty () const
	{
		return Playlist_.isEmpty ();
	}

	bool Playlist::SetProperty (const AudioSource& src, const QString& key, const QVariant& value)
	{
		const auto srcPos = std::find_if (Playlist_.begin (), Playlist_.end (),
				[&src] (const PlaylistItem& item) { return item.Source_ == src; });
		if (srcPos == Playlist_.end ())
			return false;

		srcPos->Additional_ [key] = value;
		return true;
	}
}
}
