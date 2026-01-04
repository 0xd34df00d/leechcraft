/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcoder.h"
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <util/sll/qtutil.h>
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
			return;
		}

		QTimer::singleShot (0,
				this,
				[this, files]
				{
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

					Util::NCopies (Params_.NumThreads_,
							[this] -> Util::ContextTask<void> // NOLINT(*-avoid-capturing-lambda-coroutines)
							{
								co_await Util::AddContextObject { *this };
								while (const auto maybeNextFile = co_await ToTranscode_)
									co_await TranscodeFile (*maybeNextFile);
							},
							[this] { Results_.Close (); });
				});
	}

	Util::Channel<Transcoder::Result>& Transcoder::GetResults ()
	{
		return Results_;
	}

	Util::ContextTask<void> Transcoder::TranscodeFile (const QString& origPath)
	{
		co_await Util::AddContextObject { *this };

		const auto& transcodedPath = BuildTranscodedPath (origPath, Params_);

		using namespace SyncEvents;
		const TranscodingData transcodingData { { origPath }, transcodedPath };
		emit syncEvent (XcodingStarted { transcodingData });

		QProcess ffmpeg;
		ffmpeg.start ("ffmpeg"_qs, BuildFfmpegArgs (origPath, transcodedPath, Params_));
#ifdef Q_OS_UNIX
		setpriority (PRIO_PROCESS, ffmpeg.processId (), 19);
#endif

		co_await ffmpeg;
		if (ffmpeg.exitStatus () == QProcess::NormalExit && !ffmpeg.exitCode ())
		{
			CopyTags (origPath, transcodedPath);
			emit syncEvent (XcodingFinished { transcodingData });
			Results_.Send ({ origPath, Result::Success { transcodedPath } });
		}
		else
		{
			const auto& ffmpegErr = ffmpeg.readAllStandardError ();
			qDebug () << ffmpeg.exitStatus () << ffmpeg.error () << ffmpeg.exitCode () << ffmpegErr;
			emit syncEvent (XcodingFailed { transcodingData, ffmpegErr });
			Results_.Send ({ origPath, { Util::AsLeft, Result::Failure { transcodedPath, ffmpeg.exitStatus (), ffmpegErr } } });
		}
	}
}
