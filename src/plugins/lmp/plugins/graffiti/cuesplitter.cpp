/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cuesplitter.h"
#include <functional>
#include <cmath>
#include <QTime>
#include <QThread>
#include <QFile>
#include <QtDebug>
#include <QTimer>
#include <QDir>
#include <QProcess>

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace LC::LMP::Graffiti
{
	namespace
	{
		struct Track
		{
			int Index_ = -1;

			QTime StartPos_;
			QTime EndPos_;

			QString Title_;
			QString Performer_;
		};

		struct File
		{
			QString Filename_;
			QList<Track> Tracks_;

			bool IsValid () const
			{
				return std::all_of (Tracks_.begin (), Tracks_.end (),
						[] (const Track& track) { return track.StartPos_.isValid (); });
			}
		};

		struct Cue
		{
			QString Performer_;
			QString Album_;

			int Date_ = -1;
			QString Genre_;
			QString DiscId_;

			QList<File> Files_;

			bool IsValid () const
			{
				return std::all_of (Files_.begin (), Files_.end (), [] (const File& file) { return file.IsValid (); });
			}
		};

		QString ChopQuotes (QString str)
		{
			if (str.startsWith ('\"'))
				str = str.mid (1);
			if (str.endsWith ('\"'))
				str.chop (1);
			return str;
		}

		void HandleREM (QString rem, Cue& result)
		{
			rem = rem.mid (4);

			const std::initializer_list<QPair<QStringView, std::function<void (QString)>>> setters
			{
				{ u"GENRE", [&result] (const QString& val) { result.Genre_ = val; } },
				{ u"DATE", [&result] (const QString& val) { result.Date_ = val.toInt (); } },
				{ u"DISCID", [&result] (const QString& val) { result.DiscId_ = val; } }
			};

			for (const auto& [key, setter] : setters)
				if (rem.startsWith (key))
				{
					setter (rem.mid (key.size () + 1));
					break;
				}
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
					constexpr auto HH_MM_SS_length = 8;
					const auto& components = line.mid (firstSpace + 4, HH_MM_SS_length).trimmed ().split (':');
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
		Iter HandleFile (const QString& line, Iter i, Iter end, Cue& result)
		{
			const auto startQuote = line.indexOf ('"');
			const auto endQuote = line.lastIndexOf ('"');

			File file;
			file.Filename_ = line.mid (startQuote + 1, endQuote - startQuote - 1);

			while (i != end)
			{
				if (i->startsWith ("FILE "))
					break;

				i = HandleTrack (i, end, file, result);
			}

			Track *prevTrack = nullptr;
			int index = 1;
			for (auto& track : file.Tracks_)
			{
				if (prevTrack)
					prevTrack->EndPos_ = track.StartPos_;

				prevTrack = &track;
				if (!track.Index_)
					track.Index_ = index++;
			}

			result.Files_ << file;

			return i;
		}

		Cue ParseCue (const QString& cue)
		{
			QFile file (cue);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << "unable to parse"
						<< cue;
				return {};
			}

			Cue result;
			const auto& lines = file.readAll ().split ('\n');
			for (auto i = lines.begin (), end = lines.end (); i != end; )
			{
				const auto& line = QString::fromUtf8 (i->trimmed ());

				if (constexpr QStringView marker = u"REM ";
					line.startsWith (marker))
				{
					HandleREM (line, result);
					++i;
				}
				else if (constexpr QStringView marker = u"PERFORMER ";
					line.startsWith (marker))
				{
					result.Performer_ = ChopQuotes (line.mid (marker.size ()));
					++i;
				}
				else if (constexpr QStringView marker = u"TITLE ";
					line.startsWith (marker))
				{
					result.Album_ = ChopQuotes (line.mid (marker.size ()));
					++i;
				}
				else if (constexpr QStringView marker = u"FILE ";
					line.startsWith (marker))
					i = HandleFile (line.mid (marker.size ()), i + 1, end, result);
				else
				{
					qWarning () << "unknown field"
							<< line;
					++i;
				}
			}
			return result;
		}
	}

	CueSplitter::CueSplitter (QString cueFile, QString dir, QObject *parent)
	: QObject { parent }
	, CueFile_ { std::move (cueFile) }
	, Dir_ { std::move (dir) }
	{
		QTimer::singleShot (0,
				this,
				&CueSplitter::Split);
	}

	QString CueSplitter::GetCueFile () const
	{
		return CueFile_;
	}

	namespace
	{
		QString FindFile (const QString& file, const QDir& dir)
		{
			if (dir.exists (file))
				return dir.absoluteFilePath (file);

			const auto& listing = dir.entryList (QDir::Files);

			auto candPos = std::find_if (listing.begin (), listing.end (),
					[&file] (const QString& item)
					{
						return !QString::compare (item, file, Qt::CaseInsensitive);
					});
			if (candPos != listing.end ())
				return dir.absoluteFilePath (*candPos);

			candPos = std::find_if (listing.begin (), listing.end (),
					[&file] (const QString& item)
					{
						return !QString::compare (item.section ('.', 0, -2),
								file.section ('.', 0, -2),
								Qt::CaseInsensitive);
					});

			return candPos == listing.end () ? QString () : *candPos;
		}
	}

	void CueSplitter::Split ()
	{
		const auto& cue = ParseCue (QDir (Dir_).absoluteFilePath (CueFile_));
		qDebug () << cue.IsValid () << cue.Album_ << cue.Performer_ << cue.Date_ << cue.DiscId_;
		for (const auto& file : cue.Files_)
		{
			qDebug () << "\t" << file.Filename_;
			for (const auto& track : file.Tracks_)
			{
				qDebug () << "\t\t" << track.Index_ << track.Title_ << track.StartPos_ << track.EndPos_;
			}
		}

		if (!cue.IsValid ())
		{
			emit error (tr ("Cue file is invalid"));
			deleteLater ();
			return;
		}

		for (const auto& file : cue.Files_)
		{
			const QDir dir { Dir_ };
			const auto& path = FindFile (file.Filename_, dir);
			if (path.isEmpty ())
			{
				qWarning () << file.Filename_
						<< "not found";
				emit error (tr ("No such file %1.").arg (file.Filename_));
				continue;
			}

			auto makeFilename = [&cue, &file] (const Track& track)
			{
				const auto digitsCount = static_cast<int> (std::floor (std::log10 (file.Tracks_.size ()))) + 1;

				auto filename = QString::number (track.Index_);
				if (const auto padding = digitsCount - filename.size ();
					padding > 0)
					filename = QString { padding, QChar { '0' } } + filename;

				if (!cue.Performer_.isEmpty ())
					filename += " - " + cue.Performer_;

				if (!cue.Album_.isEmpty ())
					filename += " - " + cue.Album_;

				if (!track.Title_.isEmpty ())
					filename += " - " + track.Title_;

				return filename + ".flac";
			};

			for (const auto& track : file.Tracks_)
				SplitQueue_.append ({
						path,
						dir.absoluteFilePath (makeFilename (track)),
						track.Index_,
						track.StartPos_,
						track.EndPos_,
						track.Performer_,
						cue.Album_,
						track.Title_,
						cue.Date_,
						cue.Genre_,
						cue.DiscId_
					});
		}

		TotalItems_ = SplitQueue_.size ();

		const int concurrency = std::max (QThread::idealThreadCount (), 1);
		for (int i = 0, cnt = std::min (static_cast<int> (SplitQueue_.size ()), concurrency); i < cnt; ++i)
			ScheduleNext ();
	}

	void CueSplitter::ScheduleNext ()
	{
		emit splitProgress (TotalItems_ - SplitQueue_.size () - CurrentProcesses_.size (), TotalItems_);

		if (SplitQueue_.isEmpty ())
		{
			if (CurrentProcesses_.isEmpty ())
			{
				deleteLater ();
				emit finished ();
			}
			return;
		}

		const auto& item = SplitQueue_.takeFirst ();

		auto makeTime = [] (QTime time)
		{
			return QStringLiteral ("%1:%2%3%4")
					.arg (time.hour () * 60 + time.minute ())
					.arg (time.second ())
					.arg (QLocale::system ().decimalPoint ())
					.arg (time.msec () / 10);
		};

		QStringList args { "-8", "-s" };
		if (item.From_ != QTime (0, 0))
			args << ("--skip=" + makeTime (item.From_));
		if (item.To_.isValid ())
			args << ("--until=" + makeTime (item.To_));

		auto addTag = [&args] (QStringView name, const QString& value)
		{
			if (!value.isEmpty ())
				args << ("--tag=" + name.toString () + "=" + value);
		};
		addTag (u"ARTIST", item.Artist_);
		addTag (u"ALBUM", item.Album_);
		addTag (u"TITLE", item.Title_);
		addTag (u"TRACKNUMBER", QString::number (item.Index_));
		addTag (u"GENRE", item.Genre_);
		addTag (u"DATE", item.Date_ > 0 ? QString::number (item.Date_) : QString ());
		addTag (u"DISCID", item.DiscId_);

		args << item.SourceFile_ << QStringLiteral ("-o") << item.TargetFile_;

		auto process = new QProcess (this);
		process->start (QStringLiteral ("flac"), args);

		CurrentProcesses_ << process;

		connect (process,
				&QProcess::finished,
				this,
				[this, process]
				{
					process->deleteLater ();
					CurrentProcesses_.remove (process);
					ScheduleNext ();
				});
		connect (process,
				&QProcess::errorOccurred,
				this,
				[this, process] { HandleProcessError (*process); });

#ifdef Q_OS_UNIX
		// PRIO_MAX is actually the highest _niceness_ value,
		// so it's the lowest priority.
		setpriority (PRIO_PROCESS, process->processId (), PRIO_MAX - 1);
#endif
	}

	void CueSplitter::HandleProcessError (QProcess& process)
	{
		process.deleteLater ();

		const auto& errorString = tr ("Failed to start recoder: %1.")
				.arg (process.errorString ());

		if (!EmittedErrors_.contains (errorString))
		{
			emit error (errorString);
			EmittedErrors_ << errorString;
		}

		CurrentProcesses_.remove (&process);
		ScheduleNext ();
	}
}
