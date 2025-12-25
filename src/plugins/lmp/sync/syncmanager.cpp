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
	Util::ContextTask<void> SyncManager::RunUpload (ISyncPlugin *syncer,
			QString mount, QStringList files, TranscodingParams params)
	{
		co_await Util::AddContextObject { *this };

		Transcoder transcoder { files, params };
		while (const auto transcoded = co_await transcoder.GetResults ())
			co_await UploadTranscoded (*transcoded, syncer, mount, params);
	}

	namespace
	{
		QString FixMask (const MediaInfo& info, const QString& mask)
		{
			auto result = PerformSubstitutions (mask, info, SubstitutionFlag::SFSafeFilesystem);
			const auto& ext = QFileInfo { info.LocalPath_ }.suffix ();
			if (!result.endsWith (ext))
				result += "." + ext;
			return result;
		}
	}

	Util::ContextTask<void> SyncManager::UploadTranscoded (Transcoder::Result result,
			ISyncPlugin *syncer, QString mount, TranscodingParams params)
	{
		const auto& transcoded = (co_await Util::WithHandler (result.Transcoded_,
				[] (const Transcoder::Result::Failure& failure)
				{
					// TODO log failure
				})).TargetPath_;

		const auto& mediaInfo = co_await Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded);
		const auto& targetPath = FixMask (mediaInfo, params.FilePattern_);
		const auto uploadResult = co_await syncer->Upload ({
					.LocalPath_ = transcoded,
					.OriginalLocalPath_ = result.OrigPath_,
					.Target_ = mount,
					.TargetRelPath_ = targetPath,
				});

		if (transcoded != result.OrigPath_)
			QFile::remove (transcoded);
	}
}
