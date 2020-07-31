/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downloadhelpers.h"
#include <QFuture>
#include <QFile>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Util
{
	std::optional<QFuture<TempResultType_t>> DownloadAsTemporary (IEntityManager *iem,
			const QUrl& url, DownloadParams params)
	{
		const auto& path = Util::GetTemporaryName ();
		auto e = Util::MakeEntity (url,
				path,
				DoNotSaveInHistory |
					Internal |
					DoNotNotifyUser |
					NotPersistent);
		e.Mime_ = std::move (params.Mime_);
		e.Additional_ = std::move (params.Additional_);

		auto res = iem->DelegateEntity (e);
		if (!res)
		{
			qWarning () << Q_FUNC_INFO
					<< "delegation failed for"
					<< url;
			return {};
		}

		return Util::Sequence (params.Context_, res.DownloadResult_) >>
				Util::Visitor
				{
					[] (const IDownload::Error& err) { return Util::MakeReadyFuture (TempResultType_t::Left (err)); },
					[path] (IDownload::Success)
					{
						QFile file { path };
						auto removeGuard = Util::MakeScopeGuard ([&file] { file.remove (); });
						if (!file.open (QIODevice::ReadOnly))
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to open downloaded file"
									<< file.errorString ();
							return Util::MakeReadyFuture (TempResultType_t::Left ({
									IDownload::Error::Type::LocalError,
									"unable to open file"
							}));
						}

						return Util::MakeReadyFuture (TempResultType_t::Right (file.readAll ()));
					}
				};
	}

}
