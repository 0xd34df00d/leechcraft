/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "cuesplitter.h"
#include <QTime>
#include <QFile>
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	namespace
	{
		struct Track
		{
			int Index_;

			QTime StartPos_;
			QTime EndPos_;

			QString Title_;
			QString Performer_;
		};

		struct File
		{
			QString Filename_;
			QList<Track> Tracks_;
		};

		struct Cue
		{
			QString Performer_;
			QString Album_;

			int Date_;
			QString Genre_;

			QList<File> Files_;
		};

		QString ChopQuotes (QString str)
		{
			if (str.startsWith ('\"'))
				str = str.mid (1);
			if (str.endsWith ('\"'))
				str.chop (1);
			return str;
		}

		void HandleREM (QByteArray rem, Cue& result)
		{
			rem = rem.mid (4);
			if (rem.startsWith ("GENRE "))
				result.Genre_ = rem.mid (6);
			else if (rem.startsWith ("DATE "))
				result.Date_ = rem.mid (5).toInt ();
		}

		template<typename Iter>
		Iter HandleTrack (Iter i, Iter end, File& file, const Cue& cue)
		{
			const auto& trackLine = i->trimmed ();
			if (!trackLine.startsWith ("TRACK "))
				return end;

			Track track;
			track.Performer_ = cue.Performer_;
			track.Index_ = trackLine.split (' ').value (1).toInt ();

			++i;

			while (i != end)
			{
				const auto& line = i->trimmed ();
				if (line.startsWith ("TRACK "))
					break;

				const auto firstSpace = line.indexOf (' ');
				const auto& command = line.left (firstSpace);

				auto textVal = [&line, firstSpace] () { return ChopQuotes (line.mid (firstSpace + 1).trimmed ()); };
				if (command == "TITLE")
					track.Title_ = textVal ();
				else if (command == "PERFORMER")
					track.Performer_ = textVal ();
				else if (command == "INDEX" && line.mid (firstSpace + 1, 2) == "01")
				{
					const auto& components = line.mid (firstSpace + 4, 8).trimmed ().split (':');
					if (components.size () == 3)
						track.StartPos_.setHMS (0,
								components.at (0).toInt (),
								components.at (1).toInt (),
								1000. / 75. * components.at (2).toInt ());
				}

				++i;
			}

			file.Tracks_ << track;

			return i;
		}

		template<typename Iter>
		Iter HandleFile (const QByteArray& line, Iter i, Iter end, Cue& result)
		{
			const auto startQuote = line.indexOf ('"');
			const auto endQuote = line.lastIndexOf ('"');

			File file;
			file.Filename_ = QString::fromUtf8 (line.mid (startQuote + 1, startQuote - endQuote - 2));

			while (i != end)
			{
				if (i->startsWith ("FILE "))
					break;

				i = HandleTrack (i, end, file, result);
			}

			Track *prevTrack = 0;
			for (auto& track : file.Tracks_)
			{
				if (prevTrack)
					prevTrack->EndPos_ = track.StartPos_;

				prevTrack = &track;
			}

			result.Files_ << file;

			return i;
		}

		Cue ParseCue (const QString& cue)
		{
			QFile file (cue);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse"
						<< cue;
				return Cue ();
			}

			Cue result;
			const auto& lines = file.readAll ().split ('\n');
			for (auto i = lines.begin (), end= lines.end (); i != end; )
			{
				const auto& line = i->trimmed ();

				if (line.startsWith ("REM "))
				{
					HandleREM (line, result);
					++i;
				}
				else if (line.startsWith ("PERFORMER "))
				{
					result.Performer_ = ChopQuotes (QString::fromUtf8 (line.mid (10)));
					++i;
				}
				else if (line.startsWith ("TITLE "))
				{
					result.Album_ = ChopQuotes (QString::fromUtf8 (line.mid (6)));
					++i;
				}
				else if (line.startsWith ("FILE "))
					i = HandleFile (line.mid (5), i + 1, end, result);
			}
			return result;
		}
	}

	CueSplitter::CueSplitter (const QString& cueFile, const QString& dir, QObject *parent)
	: QObject (parent)
	{
		const auto& cue = ParseCue (cueFile);
		qDebug () << cue.Album_ << cue.Performer_ << cue.Date_;
		for (const auto& file : cue.Files_)
		{
			qDebug () << "\t" << file.Filename_;
			for (const auto& track : file.Tracks_)
			{
				qDebug () << "\t\t" << track.Index_ << track.Title_ << track.StartPos_ << track.EndPos_;
			}
		}
	}
}
}
}
