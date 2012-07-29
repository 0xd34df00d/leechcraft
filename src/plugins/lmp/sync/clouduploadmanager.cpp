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

#include "clouduploadmanager.h"
#include <algorithm>
#include <QStringList>
#include <QFileInfo>
#include <QtDebug>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "clouduploader.h"

namespace LeechCraft
{
namespace LMP
{
	CloudUploadManager::CloudUploadManager (QObject *parent)
	: SyncManagerBase (parent)
	{
	}

	void CloudUploadManager::AddFiles (ICloudStoragePlugin *cloud, const QString& account,
			const QStringList& files, const TranscodingParams& params)
	{
		std::for_each (files.begin (), files.end (),
				[this, cloud, &account] (decltype (files.front ()) file)
					{ Source2Params_ [file] = { cloud, account }; });

		SyncManagerBase::AddFiles (files, params);
	}

	void CloudUploadManager::CreateUploader (ICloudStoragePlugin *cloud)
	{
		auto up = new CloudUploader (cloud, this);
		connect (up,
				SIGNAL (startedCopying (QString)),
				this,
				SLOT (handleStartedCopying (QString)));
		connect (up,
				SIGNAL (finishedCopying ()),
				this,
				SLOT (handleFinishedCopying ()));
		Cloud2Uploaders_ [cloud] = up;
	}

	void CloudUploadManager::handleFileTranscoded (const QString& from,
			const QString& transcoded, QString)
	{
		SyncManagerBase::HandleFileTranscoded (from, transcoded);

		const auto& syncTo = Source2Params_.take (from);
		if (syncTo.Account_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "dumb transcoded file detected"
					<< from
					<< transcoded;
			return;
		}

		emit uploadLog (tr ("File %1 successfully transcoded, adding to upload queue for account %2 at service %3...")
				.arg ("<em>" + QFileInfo (from).fileName () + "</em>")
				.arg ("<em>" + syncTo.Cloud_->GetCloudName () + "</em>")
				.arg ("<em>" + syncTo.Account_) + "</em>");

		if (!Cloud2Uploaders_.contains (syncTo.Cloud_))
			CreateUploader (syncTo.Cloud_);
		Cloud2Uploaders_ [syncTo.Cloud_]->Upload ({ from != transcoded, syncTo.Account_, transcoded });
	}
}
}
