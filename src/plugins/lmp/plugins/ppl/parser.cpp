/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parser.h"
#include <optional>
#include <functional>
#include <QDateTime>
#include <QtDebug>

namespace LC
{
namespace LMP
{
namespace PPL
{
	namespace
	{
		std::optional<QPair<Media::AudioInfo, QDateTime>> ParseTrack (QStringView line)
		{
			enum
			{
				Artist,
				Album,
				Title,
				TrackNum,
				Duration,
				Rating,
				Timestamp,
				// We don't really use this field, so it's OK if it's missing, let's be a more permissive parser.
				//MZTrackID,

				FieldsCount
			};

			const auto& elems = line.split ('\t');
			if (elems.size () < FieldsCount)
			{
				qWarning () << Q_FUNC_INFO
						<< "bad format for"
						<< line
						<< "got"
						<< elems.size ()
						<< "elements:"
						<< elems;
				return {};
			}

			const int required [] = { Artist, Title, Duration, Rating, Timestamp };
			if (std::ranges::any_of (required,
					[&elems] (int field) { return elems.at (field).isEmpty (); }))
			{
				qWarning () << Q_FUNC_INFO
						<< "some required data is missing for line"
						<< line;
				return {};
			}

			if (elems.at (Rating) != u"L")
				return {};

			Media::AudioInfo info
			{
				elems.at (Artist).toString (),
				elems.at (Album).toString (),
				elems.at (Title).toString (),
				{},
				elems.at (Duration).toInt (),
				0,
				elems.at (TrackNum).toInt (),
				{}
			};

			const auto& dt = QDateTime::fromSecsSinceEpoch (elems.at (Timestamp).toLongLong ());

			return { { info, dt } };
		}
	}

	Media::IAudioScrobbler::BackdatedTracks_t ParseData (const QString& data)
	{
		Media::IAudioScrobbler::BackdatedTracks_t tracks;

		auto dateConverter = +[] (const QDateTime& dt) { return dt; };
		for (auto line : QStringView { data }.split ('\n', Qt::SkipEmptyParts))
		{
			if (line.at (0) == '#')
			{
				if (line == u"#TZ/UNKNOWN")
					dateConverter = +[] (const QDateTime& dt) { return dt.toUTC (); };
				continue;
			}

			if (auto trackInfo = ParseTrack (line))
			{
				trackInfo->second = dateConverter (trackInfo->second);
				tracks.push_back (*trackInfo);
			}
		}
		return tracks;
	}
}
}
}
