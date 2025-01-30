/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clouduploader.h"
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace LMP
{
	CloudUploader::CloudUploader (ICloudStoragePlugin *cloud, QObject *parent)
	: QObject { parent }
	, Cloud_ { cloud }
	{
	}

	void CloudUploader::Upload (const UploadJob& job)
	{
		Queue_ << job;
		if (!IsRunning_)
			DrainQueue ();
	}

	Util::ContextTask<> CloudUploader::DrainQueue ()
	{
		IsRunning_ = true;
		const auto runningGuard = Util::MakeScopeGuard ([this] { IsRunning_ = false; });

		co_await Util::AddContextObject { *this };

		while (!Queue_.isEmpty ())
		{
			const auto job = Queue_.takeFirst ();
			qDebug () << "uploading" << job.Filename_ << "to" << job.Account_;

			emit startedCopying (job.Filename_);
			const auto res = co_await Cloud_->Upload (job.Account_, job.Filename_);
			emit finishedCopying ();

			Visit (res,
				[&] (Util::Void)
				{
					if (job.RemoveOnFinish_)
						QFile::remove (job.Filename_);
				},
				[&] (const ICloudStoragePlugin::UploadError& error)
				{
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("LMP",
								tr ("Error uploading file %1 to cloud: %2.")
									.arg (QFileInfo { job.Filename_ }.fileName ())
									.arg (error.Message_),
								Priority::Critical));
				});
		}
	}
}
}
