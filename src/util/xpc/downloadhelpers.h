/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QVariantMap>
#include <util/sll/eitherfwd.h>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/idownload.h>
#include "xpcconfig.h"

class IEntityManager;

namespace LC::Util
{
	using TempDownload_t = Either<IDownload::Error, QByteArray>;

	struct DownloadParams
	{
		QString Mime_ {};
		QVariantMap Additional_ {};
	};

	UTIL_XPC_API Task<TempDownload_t> DownloadAsTemporary (IEntityManager& iem, QUrl url, DownloadParams params = {});
}
