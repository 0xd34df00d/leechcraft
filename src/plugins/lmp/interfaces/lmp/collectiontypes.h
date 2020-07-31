/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <QDateTime>

namespace LC
{
namespace LMP
{
namespace Collection
{
	struct Track
	{
		int ID_;

		int Number_;
		QString Name_;
		int Length_;
		QStringList Genres_;

		QString FilePath_;
	};

	struct Album
	{
		int ID_;

		QString Name_;
		int Year_;
		QString CoverPath_;

		QList<Track> Tracks_;
	};
	typedef std::shared_ptr<Album> Album_ptr;

	struct Artist
	{
		int ID_;

		QString Name_;
		QList<Album_ptr> Albums_;
	};
	typedef QList<Artist> Artists_t;

	struct TrackStats
	{
		int TrackID_ = 0;

		int Playcount_ = 0;
		QDateTime Added_;
		QDateTime LastPlay_;
		int Score_ = 0;
		int Rating_ = 0;

		operator bool () const;
	};
}
}
}
