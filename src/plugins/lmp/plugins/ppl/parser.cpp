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
		template<typename S>
		std::function<QDateTime (QDateTime)> GetDateConverter (S&& sect)
		{
			if (sect == "UTC")
				return [] (const QDateTime& dt) { return dt; };
			else if (sect == "UNKNOWN")
				return [] (const QDateTime& dt) { return dt.toUTC (); };

			qWarning () << Q_FUNC_INFO
					<< "unknown timezone"
					<< sect
					<< "assuming UTC";
			return [] (const QDateTime& dt) { return dt; };
		}

		std::optional<QPair<Media::AudioInfo, QDateTime>> ParseTrack (QStringRef line)
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
			if (std::any_of (std::begin (required), std::end (required),
					[&elems] (int field) { return elems.at (field).isEmpty (); }))
			{
				qWarning () << Q_FUNC_INFO
						<< "some required data is missing for line"
						<< line;
				return {};
			}

			if (elems.at (Rating) != "L")
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

		auto dateConverter = GetDateConverter (QString { "UTC" });
		for (auto line : data.splitRef ('\n', Qt::SkipEmptyParts))
		{
			if (line.at (0) == '#')
			{
				if (line.startsWith ("#TZ/"))
					dateConverter = GetDateConverter (line.split ('/').value (1));
				continue;
			}

			if (auto trackInfo = ParseTrack (std::move (line)))
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
