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

namespace LC
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

			bool IsValid () const
			{
				for (const auto& track : Tracks_)
					if (!track.StartPos_.isValid ())
						return false;
				return true;
			}
		};

		struct Cue
		{
			QString Performer_;
			QString Album_;

			int Date_;
			QString Genre_;
			QString DiscId_;

			QList<File> Files_;

			bool IsValid () const
			{
				for (const auto& file : Files_)
					if (!file.IsValid ())
						return false;
				return true;
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

			const QList<QPair<QString, std::function<void (QString)>>> setters ({
					{ "GENRE", [&result] (const QString& val) { result.Genre_ = val; } },
					{ "DATE", [&result] (const QString& val) { result.Date_ = val.toInt (); } },
					{ "DISCID", [&result] (const QString& val) { result.DiscId_ = val; } }
				});

			for (const auto& key : setters)
				if (rem.startsWith (key.first))
				{
					key.second (rem.mid (key.first.size () + 1));
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

			Track *prevTrack = 0;
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
				qWarning () << Q_FUNC_INFO
						<< "unable to parse"
						<< cue;
				return Cue ();
			}

			Cue result;
			const auto& lines = file.readAll ().split ('\n');
			for (auto i = lines.begin (), end = lines.end (); i != end; )
			{
				const auto& line = QString::fromUtf8 (i->trimmed ());

				if (line.startsWith ("REM "))
				{
					HandleREM (line, result);
					++i;
				}
				else if (line.startsWith ("PERFORMER "))
				{
					result.Performer_ = ChopQuotes (line.mid (10));
					++i;
				}
				else if (line.startsWith ("TITLE "))
				{
					result.Album_ = ChopQuotes (line.mid (6));
					++i;
				}
				else if (line.startsWith ("FILE "))
					i = HandleFile (line.mid (5), i + 1, end, result);
				else
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown field"
							<< line;
					++i;
				}
			}
			return result;
		}
	}

	CueSplitter::CueSplitter (const QString& cueFile, const QString& dir, QObject *parent)
	: QObject (parent)
	, CueFile_ (cueFile)
	, Dir_ (dir)
	{
		QTimer::singleShot (0,
				this,
				SLOT (split ()));
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

	void CueSplitter::split ()
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
				qWarning () << Q_FUNC_INFO
						<< file.Filename_
						<< "not found";
				emit error (tr ("No such file %1.").arg (file.Filename_));
				continue;
			}

			auto makeFilename = [&cue, &file] (const Track& track)
			{
				const auto digitsCount = static_cast<int> (std::floor (std::log10 (file.Tracks_.size ()))) + 1;
				auto filename = QString { "%1" }
						.arg (track.Index_,
								std::max (2, digitsCount),
								10,
								QChar { '0' });
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

		const auto concurrency = std::max (QThread::idealThreadCount (), 1);
		for (int i = 0, cnt = std::min (SplitQueue_.size (), concurrency); i < cnt; ++i)
			scheduleNext ();
	}

	void CueSplitter::scheduleNext ()
	{
		emit splitProgress (TotalItems_ - SplitQueue_.size () - CurrentlyProcessing_, TotalItems_, this);

		if (SplitQueue_.isEmpty ())
		{
			if (!CurrentlyProcessing_)
			{
				deleteLater ();
				emit finished (this);
			}
			return;
		}

		const auto& item = SplitQueue_.takeFirst ();

		auto makeTime = [] (const QTime& time)
		{
			return QString ("%1:%2%3%4")
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

		auto addTag = [&args] (const QString& name, const QString& value)
		{
			if (!value.isEmpty ())
				args << ("--tag=" + name + "=" + value);
		};
		addTag ("ARTIST", item.Artist_);
		addTag ("ALBUM", item.Album_);
		addTag ("TITLE", item.Title_);
		addTag ("TRACKNUMBER", QString::number (item.Index_));
		addTag ("GENRE", item.Genre_);
		addTag ("DATE", item.Date_ > 0 ? QString::number (item.Date_) : QString ());
		addTag ("DISCID", item.DiscId_);

		args << item.SourceFile_ << "-o" << item.TargetFile_;

		++CurrentlyProcessing_;
		auto process = new QProcess (this);
		process->start ("flac", args);

		connect (process,
				SIGNAL (finished (int)),
				this,
				SLOT (handleProcessFinished (int)));
		connect (process,
				SIGNAL (error (QProcess::ProcessError)),
				this,
				SLOT (handleProcessError ()));

#ifdef Q_OS_UNIX
		setpriority (PRIO_PROCESS, process->processId (), 19);
#endif
	}

	void CueSplitter::handleProcessFinished (int)
	{
		sender ()->deleteLater ();

		--CurrentlyProcessing_;
		scheduleNext ();
	}

	void CueSplitter::handleProcessError ()
	{
		auto process = qobject_cast<QProcess*> (sender ());
		process->deleteLater ();

		const auto& errorString = tr ("Failed to start recoder: %1.")
				.arg (process->errorString ());

		if (!EmittedErrors_.contains (errorString))
		{
			emit error (errorString);
			EmittedErrors_ << errorString;
		}

		--CurrentlyProcessing_;
		scheduleNext ();
	}
}
}
}
