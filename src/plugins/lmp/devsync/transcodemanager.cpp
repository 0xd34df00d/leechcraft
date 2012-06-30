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

#include "transcodemanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include "transcodejob.h"

namespace LeechCraft
{
namespace LMP
{
	TranscodeManager::TranscodeManager (QObject *parent)
	: QObject (parent)
	{
	}

	void TranscodeManager::Enqueue (const QStringList& files, const TranscodingParams& params)
	{
		std::transform (files.begin (), files.end (), std::back_inserter (Queue_),
				[&params] (decltype (files.front ()) file) { return qMakePair (file, params); });

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
		if (!Queue_.isEmpty ())
		{
			const auto& pair = Queue_.takeFirst ();
			EnqueueJob (pair);
		}

		if (success)
			emit fileReady (job->GetOrigPath (), job->GetTranscodedPath (), job->GetTargetPattern ());
	}
}
}
