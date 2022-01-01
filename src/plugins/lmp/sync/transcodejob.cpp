/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcodejob.h"
#include <stdexcept>
#include <functional>
#include <QMap>
#include <QDir>
#include <QUuid>
#include <QtDebug>
#include <taglib/tag.h>
#include "transcodingparams.h"
#include "core.h"
#include "localfileresolver.h"

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#endif

namespace LC
{
namespace LMP
{
	namespace
	{
		QString BuildTranscodedPath (const QString& path, const TranscodingParams& params)
		{
			static const auto tmpDirName = []
			{
#ifdef Q_OS_UNIX
				return QString { "lmp_transcode_%1" }
						.arg (getuid ());
#else
				return "lmp_transcode";
#endif
			} ();

			QDir dir = QDir::temp ();
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
	}

	TranscodeJob::TranscodeJob (const QString& path, const TranscodingParams& params, QObject* parent)
	: QObject (parent)
	, Process_ (new QProcess (this))
	, OriginalPath_ (path)
	, TranscodedPath_ (BuildTranscodedPath (path, params))
	, TargetPattern_ (params.FilePattern_)
	{
		QStringList args
		{
			"-i",
			path,
			"-vn"
		};
		args << Formats {}.GetFormat (params.FormatID_)->ToFFmpeg (params);
		args << TranscodedPath_;

		connect (Process_,
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SLOT (handleFinished (int, QProcess::ExitStatus)));
		connect (Process_,
				SIGNAL (readyRead ()),
				this,
				SLOT (handleReadyRead ()));
		Process_->start ("ffmpeg", args);

#ifdef Q_OS_UNIX
		setpriority (PRIO_PROCESS, Process_->processId (), 19);
#endif
	}

	QString TranscodeJob::GetOrigPath () const
	{
		return OriginalPath_;
	}

	QString TranscodeJob::GetTranscodedPath () const
	{
		return TranscodedPath_;
	}

	QString TranscodeJob::GetTargetPattern () const
	{
		return TargetPattern_;
	}

	namespace
	{
		bool CheckTags (const TagLib::FileRef& ref, const QString& filename)
		{
			if (!ref.tag ())
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot get tags for"
						<< filename;
				return false;
			}

			return true;
		}

		void CopyTags (const QString& from, const QString& to)
		{
			const auto resolver = Core::Instance ().GetLocalFileResolver ();

			QMutexLocker locker (&resolver->GetMutex ());

			auto fromRef = resolver->GetFileRef (from);
			auto toRef = resolver->GetFileRef (to);

			if (!CheckTags (fromRef, from) || !CheckTags (toRef, to))
				return;

			TagLib::Tag::duplicate (fromRef.tag (), toRef.tag ());

			if (!toRef.save ())
				qWarning () << Q_FUNC_INFO
						<< "cannot save file"
						<< to;
		}
	}

	void TranscodeJob::handleFinished (int code, QProcess::ExitStatus status)
	{
		qDebug () << Q_FUNC_INFO << code << status;
		if (code)
			qWarning () << Q_FUNC_INFO
					<< Process_->readAllStandardError ();

		CopyTags (OriginalPath_, TranscodedPath_);

		emit done (this, !code);
	}

	void TranscodeJob::handleReadyRead ()
	{
	}
}
}
