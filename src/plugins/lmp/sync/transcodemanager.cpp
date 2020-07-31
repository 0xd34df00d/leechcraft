/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcodemanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include <util/sll/prelude.h>
#include "transcodejob.h"

namespace LC
{
namespace LMP
{
	TranscodeManager::TranscodeManager (QObject *parent)
	: QObject (parent)
	{
	}

	namespace
	{
		bool IsLossless (const QString& filename)
		{
			return filename.endsWith (".flac", Qt::CaseInsensitive) ||
					filename.endsWith (".alac", Qt::CaseInsensitive);
		}
	}

	void TranscodeManager::Enqueue (QStringList files, const TranscodingParams& params)
	{
		if (params.FormatID_.isEmpty ())
		{
			for (const auto& file : files)
				emit fileReady (file, file, params.FilePattern_);
			return;
		}

		if (params.OnlyLossless_)
		{
			const auto partPos = std::stable_partition (files.begin (), files.end (), IsLossless);

			for (auto i = partPos; i != files.end (); ++i)
				emit fileReady (*i, *i, params.FilePattern_);

			files.erase (partPos, files.end ());
		}

		Queue_ += Util::Map (files,
				[&params] (const QString& file) { return qMakePair (file, params); });

		while (RunningJobs_.size () < params.NumThreads_ && !Queue_.isEmpty ())
			EnqueueJob (Queue_.takeFirst ());
	}

	void TranscodeManager::EnqueueJob (const QPair<QString, TranscodingParams>& pair)
	{
		auto job = new TranscodeJob (pair.first, pair.second, this);
		RunningJobs_ << job;
		connect (job,
				SIGNAL (done (TranscodeJob*, bool)),
				this,
				SLOT (handleDone (TranscodeJob*, bool)));
		emit fileStartedTranscoding (QFileInfo (pair.first).fileName ());
	}

	void TranscodeManager::handleDone (TranscodeJob *job, bool success)
	{
		RunningJobs_.removeAll (job);
		job->deleteLater ();

		if (!Queue_.isEmpty ())
			EnqueueJob (Queue_.takeFirst ());

		if (success)
			emit fileReady (job->GetOrigPath (), job->GetTranscodedPath (), job->GetTargetPattern ());
		else
			emit fileFailed (job->GetOrigPath ());
	}
}
}
