/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncmanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include <util/sll/either.h>
#include <util/threads/coro.h>
#include <util/lmp/util.h>
#include "transcoder.h"
#include "../core.h"
#include "../localfileresolver.h"

namespace LC::LMP
{
	Util::ContextTask<void> SyncManager::RunUpload (QStringList files, TranscodingParams params, Context context)
	{
		co_await Util::AddContextObject { *this };

		Transcoder transcoder { files, params };
		while (const auto transcoded = co_await transcoder.GetResults ())
			co_await UploadTranscoded (*transcoded, context);
	}

	Util::ContextTask<void> SyncManager::UploadTranscoded (Transcoder::Result result, Context context)
	{
		const auto& transcoded = (co_await Util::WithHandler (result.Transcoded_,
				[] (const Transcoder::Result::Failure& failure)
				{
					// TODO log failure
				})).TargetPath_;
		qDebug () << "uploading" << transcoded;

		const auto& mediaInfo = co_await Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded);
		const auto uploadResult = co_await context.Syncer_->Upload ({
					.LocalPath_ = transcoded,
					.OriginalLocalPath_ = result.OrigPath_,
					.MediaInfo_ = mediaInfo,
					.Target_ = context.Target_,
					.Config_ = context.Config_,
				});

		if (transcoded != result.OrigPath_)
			QFile::remove (transcoded);
	}
}
