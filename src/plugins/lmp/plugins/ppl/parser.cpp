/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "parser.h"
#include <boost/optional.hpp>
#include <functional>
#include <QDateTime>
#include <QtDebug>

namespace LeechCraft
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

		template<typename S>
		boost::optional<QPair<Media::AudioInfo, QDateTime>> ParseTrack (S&& line)
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

			const auto& dt = QDateTime::fromTime_t (elems.at (Timestamp).toLongLong ());

			return { { info, dt } };
		}
	}

	Media::IAudioScrobbler::BackdatedTracks_t ParseData (const QString& data)
	{
		Media::IAudioScrobbler::BackdatedTracks_t tracks;

		auto dateConverter = GetDateConverter (QString { "UTC" });
		for (auto line : data.splitRef ('\n', QString::SkipEmptyParts))
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
