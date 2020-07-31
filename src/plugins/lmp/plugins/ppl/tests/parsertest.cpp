/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parsertest.h"
#include <QtTest>
#include "parser.cpp"

QTEST_APPLESS_MAIN (LC::LMP::PPL::ParserTest)

QDebug operator<< (QDebug dbg, const Media::AudioInfo& info)
{
	dbg.nospace () << "Media::AudioInfo { "
			<< info.Artist_ << "; "
			<< info.Album_ << "; "
			<< info.Title_ << "; "
			<< info.Genres_ << "; "
			<< info.Length_ << "; "
			<< info.Year_ << "; "
			<< info.TrackNumber_ << "; "
			<< info.Other_ << " }";
	return dbg.space ();
}

namespace QTest
{
	template<>
	char* toString (const Media::IAudioScrobbler::BackdatedTracks_t::value_type& pair)
	{
		QString result;
		QDebug { &result } << pair;
		return qstrdup (result.toUtf8 ().constData ());
	}
}

namespace LC
{
namespace LMP
{
namespace PPL
{
	void ParserTest::testBasicParsing ()
	{
		const QString data = R"(
#TZ/UNKNOWN
Heart Of A Coward	Severance	Monstro	1	213	L	1470071135
)";

		const auto& result = ParseData (data);

		const Media::IAudioScrobbler::BackdatedTracks_t expected
		{
			{
				Media::AudioInfo
				{
					"Heart Of A Coward",
					"Severance",
					"Monstro",
					{},
					213,
					0,
					1,
					{}
				},
				QDateTime::fromSecsSinceEpoch (1470071135).toUTC ()
			}
		};

		QCOMPARE (result, expected);
	}

	void ParserTest::testSkipping ()
	{
		const QString data = R"(
#AUDIOSCROBBLER/1.1
#TZ/UNKNOWN
#CLIENT/Rockbox sansaclipzip $Revision$
Heart Of A Coward	Severance	Monstro	1	213	S	1470071129
Heart Of A Coward	Severance	Monstro	1	213	L	1470071135
Heart Of A Coward	Severance	Prey	2	228	L	1470071375
Heart Of A Coward	Severance	Distance	3	238	S	470071603
Heart Of A Coward	Severance	Nauseam	4	218	L	1470072028
)";

		const auto& result = ParseData (data);

		const Media::IAudioScrobbler::BackdatedTracks_t expected
		{
			{
				Media::AudioInfo
				{
					"Heart Of A Coward",
					"Severance",
					"Monstro",
					{},
					213,
					0,
					1,
					{}
				},
				QDateTime::fromSecsSinceEpoch (1470071135).toUTC ()
			},
			{
				Media::AudioInfo
				{
					"Heart Of A Coward",
					"Severance",
					"Prey",
					{},
					228,
					0,
					2,
					{}
				},
				QDateTime::fromSecsSinceEpoch (1470071375).toUTC ()
			},
			{
				Media::AudioInfo
				{
					"Heart Of A Coward",
					"Severance",
					"Nauseam",
					{},
					218,
					0,
					4,
					{}
				},
				QDateTime::fromSecsSinceEpoch (1470072028).toUTC ()
			}
		};

		QCOMPARE (result, expected);
	}

	void ParserTest::testSkipMissingFieldsRecords ()
	{
		const QString data = R"(
#AUDIOSCROBBLER/1.1
#TZ/UNKNOWN
#CLIENT/Rockbox sansaclipzip $Revision$
Heart Of A Coward	Severance	Monstro	213	S	1470071129
)";

		const auto& result = ParseData (data);
		const Media::IAudioScrobbler::BackdatedTracks_t expected {};

		QCOMPARE (result, expected);
	}

	void ParserTest::testIgnoreMissingOptionalFields ()
	{
		const QString data = R"(
#TZ/UNKNOWN
Heart Of A Coward		Monstro		213	L	1470071135
)";

		const auto& result = ParseData (data);

		const Media::IAudioScrobbler::BackdatedTracks_t expected
		{
			{
				Media::AudioInfo
				{
					"Heart Of A Coward",
					{},
					"Monstro",
					{},
					213,
					0,
					0,
					{}
				},
				QDateTime::fromSecsSinceEpoch (1470071135).toUTC ()
			}
		};

		QCOMPARE (result, expected);
	}
}
}
}
