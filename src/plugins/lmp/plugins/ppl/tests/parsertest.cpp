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

#include "parsertest.h"
#include <QtTest>
#include "parser.cpp"

QTEST_MAIN (LeechCraft::LMP::PPL::ParserTest)

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

namespace LeechCraft
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
				QDateTime::fromTime_t (1470071135).toUTC ()
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
				QDateTime::fromTime_t (1470071135).toUTC ()
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
				QDateTime::fromTime_t (1470071375).toUTC ()
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
				QDateTime::fromTime_t (1470072028).toUTC ()
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
				QDateTime::fromTime_t (1470071135).toUTC ()
			}
		};

		QCOMPARE (result, expected);
	}
}
}
}
