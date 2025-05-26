/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downloadhelpers.h"
#include <QFile>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include <util/threads/coro/future.h>
#include <util/threads/coro.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Util
{
	Task<TempDownload_t> DownloadAsTemporary (IEntityManager& iem, QUrl url, DownloadParams params)
	{
		const auto& path = GetTemporaryName ();
		auto e = MakeEntity (url,
				path,
				DoNotSaveInHistory |
					Internal |
					DoNotNotifyUser |
					NotPersistent);
		e.Mime_ = std::move (params.Mime_);
		e.Additional_ = std::move (params.Additional_);

		const auto res = iem.DelegateEntity (e);
		if (!res)
		{
			qWarning () << "delegation failed for" << url;
			co_return IDownload::Error
			{
				IDownload::Error::Type::LocalError,
				QObject::tr ("Unable to find a downloader plugin.")
			};
		}

		const auto result = co_await res.DownloadResult_;
		const auto success [[maybe_unused]] = co_await WithHandler (result,
				[&url] (const IDownload::Error& error)
				{
					qWarning () << "failed downloading" << url << static_cast<int> (error.Type_) << error.Message_;
				});

		QFile file { path };
		auto removeGuard = MakeScopeGuard ([&file] { file.remove (); });
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << "unable to open downloaded file" << file.errorString ();
			co_return IDownload::Error
			{
				IDownload::Error::Type::LocalError,
				QObject::tr ("Unable to open local file: %1.").arg (file.errorString ())
			};
		}

		co_return file.readAll ();
	}
}
