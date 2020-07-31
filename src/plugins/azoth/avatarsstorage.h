/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QObject>
#include <QCache>
#include "interfaces/azoth/ihaveavatars.h"

template<typename>
class QFuture;

namespace LC
{
namespace Azoth
{
	class ICLEntry;
	class AvatarsStorageThread;

	using MaybeImage = std::optional<QImage>;
	using MaybeByteArray = std::optional<QByteArray>;

	class AvatarsStorage : public QObject
	{
		AvatarsStorageThread * const StorageThread_;

		using CacheKey_t = QPair<QString, IHaveAvatars::Size>;
		using CacheValue_t = std::variant<QByteArray, QImage>;
		QCache<CacheKey_t, CacheValue_t> Cache_;
	public:
		explicit AvatarsStorage (QObject* = nullptr);

		QFuture<void> SetAvatar (const QString&, IHaveAvatars::Size, const QImage&);
		QFuture<void> SetAvatar (const QString&, IHaveAvatars::Size, const QByteArray&);
		QFuture<MaybeImage> GetAvatar (const ICLEntry*, IHaveAvatars::Size);
		QFuture<MaybeByteArray> GetAvatar (const QString&, IHaveAvatars::Size);

		QFuture<void> DeleteAvatars (const QString&);

		void SetCacheSize (int mibs);
	};
}
}
