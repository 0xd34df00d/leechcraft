/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QVariantMap>
#include <QSet>
#include "../engine/audiosource.h"

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	struct PlaylistItem
	{
		AudioSource Source_;
		QVariantMap Additional_;

		explicit PlaylistItem (const AudioSource&);
		PlaylistItem (const AudioSource&, const QVariantMap&);
		PlaylistItem (const AudioSource&, const MediaInfo&);

		std::optional<MediaInfo> GetMediaInfo () const;
	};

	class Playlist
	{
		typedef QList<PlaylistItem> Container_t;
		Container_t Playlist_;

		QSet<QUrl> UrlsSet_;
	public:
		typedef Container_t::const_iterator const_iterator;
		typedef Container_t::iterator iterator;

		Playlist () = default;
		Playlist (const QList<PlaylistItem>&);
		explicit Playlist (const QList<AudioSource>&);

		const_iterator begin () const;
		iterator begin ();
		const_iterator end () const;
		iterator end ();

		iterator erase (iterator);

		Playlist& Append (const PlaylistItem&);
		Playlist& operator+= (const AudioSource&);
		Playlist& operator+= (const Playlist&);

		QList<AudioSource> ToSources () const;

		bool IsEmpty () const;

		bool SetProperty (const AudioSource&, const QString&, const QVariant&);
	};

	struct RawReadData
	{
		QString SourceStr_;
		QVariantMap Additional_;
	};
}
}
