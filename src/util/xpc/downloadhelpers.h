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
#include <interfaces/idownload.h>
#include "xpcconfig.h"

template<typename>
class QFuture;

class IEntityManager;

namespace LC::Util
{
	using TempResultType_t = Util::Either<IDownload::Error, QByteArray>;

	struct DownloadParams
	{
		QString Mime_ {};
		QVariantMap Additional_ {};
		QObject *Context_ = nullptr;
	};

	UTIL_XPC_API std::optional<QFuture<TempResultType_t>> DownloadAsTemporary (IEntityManager *iem,
			const QUrl& url, DownloadParams params = {});
}
