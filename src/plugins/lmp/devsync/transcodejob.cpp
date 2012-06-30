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

#include "transcodejob.h"
#include <stdexcept>
#include <functional>
#include <QMap>
#include <QDir>
#include <QtDebug>
#include "transcodingparams.h"

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		QStringList OggParams (const TranscodingParams& params)
		{
			return { "-acodec", "libvorbis", "-aq", QString::number (params.Quality_) };
		}
	}

	TranscodeJob::TranscodeJob (const QString& path, const TranscodingParams& params, QObject* parent)
	: QObject (parent)
	, Process_ (new QProcess (this))
	, OriginalPath_ (path)
	, TargetPattern_ (params.FilePattern_)
	{
		QMap<QString, std::function<QStringList (TranscodingParams)>> trans;
		trans ["ogg"] = OggParams;

		QDir dir = QDir::temp ();
		if (!dir.exists ("lmp_transcode"))
			dir.mkdir ("lmp_transcode");
		if (!dir.cd ("lmp_transcode"))
			throw std::runtime_error ("unable to cd into temp dir");

		const QFileInfo fi (path);
		TranscodedPath_ = dir.absoluteFilePath (fi.fileName () + '.' + params.Format_);

		QStringList args;
		args << "-i" << path;
		args << trans [params.Format_] (params);
		args << "-map_metadata" << "0";
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
		setpriority (PRIO_PROCESS, Process_->pid (), 19);
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

	void TranscodeJob::handleFinished (int code, QProcess::ExitStatus status)
	{
		qDebug () << Q_FUNC_INFO << code << status;
		emit done (this, !code);
	}

	void TranscodeJob::handleReadyRead ()
	{
	}
}
}
