/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcoder.h"
#include <functional>
#include <optional>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sll/visitor.h>
#include <util/threads/coro.h>
#include <util/threads/coro/inparallel.h>
#include <taglib/tag.h>
#include "core.h"
#include "localfileresolver.h"

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#endif

namespace LC::LMP
{
	namespace
	{
		QStringList BuildFfmpegArgs (const QString& sourcePath, const QString& transcodedPath, const TranscodingParams& params)
		{
			QStringList args
			{
				"-i",
				sourcePath,
				"-vn"
			};
			args << Formats {}.GetFormat (params.FormatID_)->ToFFmpeg (params);
			args << transcodedPath;
			return args;
		}

		QString BuildTranscodedPath (const QString& path, const TranscodingParams& params)
		{
			static const auto tmpDirName = []
			{
#ifdef Q_OS_UNIX
				return "lmp_transcode_%1"_qs.arg (getuid ());
#else
				return "lmp_transcode";
#endif
			} ();

			auto dir = QDir::temp ();
			if (!dir.exists (tmpDirName))
				dir.mkdir (tmpDirName);
			if (!dir.cd (tmpDirName))
				throw std::runtime_error ("unable to cd into temp dir");

			const QFileInfo fi (path);

			const auto format = Formats ().GetFormat (params.FormatID_);

			auto result = dir.absoluteFilePath (fi.fileName ());
			auto ext = format->GetFileExtension ();
			ext.prepend (QUuid::createUuid ().toString () + ".");
			const auto dotIdx = result.lastIndexOf ('.');
			if (dotIdx == -1)
				result += '.' + ext;
			else
				result.replace (dotIdx + 1, result.size () - dotIdx, ext);

			return result;
		}

		bool IsLossless (const QString& filename)
		{
			return filename.endsWith (".flac"_ql, Qt::CaseInsensitive) ||
					filename.endsWith (".alac"_ql, Qt::CaseInsensitive);
		}

		bool CheckTags (const TagLib::FileRef& ref, const QString& filename)
		{
			if (!ref.tag ())
			{
				qWarning () << "cannot get tags for" << filename;
				return false;
			}
			return true;
		}

		void CopyTags (const QString& from, const QString& to)
		{
			const auto resolver = Core::Instance ().GetLocalFileResolver ();

			QMutexLocker locker { &resolver->GetMutex () };

			const auto fromRef = resolver->GetFileRef (from);
			auto toRef = resolver->GetFileRef (to);

			if (!CheckTags (fromRef, from) || !CheckTags (toRef, to))
				return;

			TagLib::Tag::duplicate (fromRef.tag (), toRef.tag ());

			if (!toRef.save ())
				qWarning () << "cannot save file" << to;
		}
	}

	Transcoder::Transcoder (const QStringList& files, const TranscodingParams& params)
	: Params_ { params }
	{
		if (params.FormatID_.isEmpty ())
		{
			for (const auto& file : files)
				Results_.Send ({ file, Result::Success { file } });
			Results_.Close ();
			return;
		}

		Run (files);
	}

	Util::Channel<Transcoder::Result>& Transcoder::GetResults ()
	{
		return Results_;
	}

	Util::ContextTask<void> Transcoder::Run (QStringList files)
	{
		using namespace std::chrono_literals;

		co_await Util::AddContext { CoroContext_ };
		co_await 0ms;		// effectively QTimer::singleShot to allow consumers connecting to the signals emitted here

		int skipped = 0;
		for (const auto& file : files)
		{
			if (Params_.OnlyLossless_ && !IsLossless (file))
			{
				Results_.Send ({ file, Result::Success { file } });
				++skipped;
			}
			else
				ToTranscode_.Send (file);
		}
		emit syncEvent (SyncEvents::XcodingSkipped { skipped });

		ToTranscode_.Close ();

		const auto resultsGuard = Util::MakeScopeGuard ([this] { Results_.Close (); });
		co_await Util::NCopies (Params_.NumThreads_, std::bind_front (&Transcoder::DrainTranscodeQueue, this));
	}

	Util::ContextTask<void> Transcoder::DrainTranscodeQueue ()
	{
		co_await Util::AddContext { CoroContext_ };
		while (const auto maybeNextFile = co_await ToTranscode_)
			co_await TranscodeFile (*maybeNextFile);
	}

	Util::ContextTask<void> Transcoder::TranscodeFile (const QString& origPath)
	{
		co_await Util::AddContext { CoroContext_ };

		const auto& transcodedPath = BuildTranscodedPath (origPath, Params_);

		using namespace SyncEvents;
		const TranscodingData transcodingData { { origPath }, transcodedPath };
		emit syncEvent (XcodingStarted { transcodingData });

		auto removePartialOutput = Util::MakeScopeGuard ([&transcodedPath] { QFile::remove (transcodedPath); });

		QProcess ffmpeg;
#ifdef Q_OS_UNIX
		ffmpeg.setChildProcessModifier ([] { setpriority (PRIO_PROCESS, 0, 19); });
#endif
		ffmpeg.start ("ffmpeg"_qs, BuildFfmpegArgs (origPath, transcodedPath, Params_));

		const auto outcome = co_await ffmpeg;
		if (outcome == Util::ProcessOutcome { Util::ProcessExited { .Code_ = 0 } })
		{
			removePartialOutput.Dismiss ();
			CopyTags (origPath, transcodedPath);
			emit syncEvent (XcodingFinished { transcodingData });
			Results_.Send ({ origPath, Result::Success { transcodedPath } });
			co_return;
		}

		const auto errorText = Util::Visit (outcome,
				[] (const Util::ProcessExited& err) { return tr ("ffmpeg exited with code %1.").arg (err.Code_); },
				[] (const Util::ProcessCrashed& err) { return tr ("ffmpeg crashed with code %1.").arg (err.Code_); },
				[] (const Util::ProcessFailedToStart& err) { return tr ("ffmpeg failed to start: %1").arg (err.Error_); });

		const auto& stderrText = QString::fromUtf8 (ffmpeg.readAllStandardError ()).trimmed ();
		const auto& message = stderrText.isEmpty () ? errorText : errorText + u'\n' + stderrText;
		qWarning () << "transcoding failed for" << origPath << outcome << stderrText;
		emit syncEvent (XcodingFailed { transcodingData, message });
		Results_.Send ({ origPath, { Util::AsLeft, Result::Failure { message } } });
	}
}
