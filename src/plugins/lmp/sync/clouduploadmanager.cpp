/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clouduploadmanager.h"
#include <algorithm>
#include <QStringList>
#include <QFileInfo>
#include <QtDebug>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "clouduploader.h"

namespace LC
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
				[this, cloud, &account] (const auto& file) { Source2Params_ [file] = { cloud, account }; });

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
