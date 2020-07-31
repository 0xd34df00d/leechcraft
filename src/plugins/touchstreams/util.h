/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

template<typename K, typename V>
class QMap;

using QVariantMap = QMap<QString, QVariant>;

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace Util::SvcAuth
{
	class VkAuthManager;
}

namespace TouchStreams
{
	bool CheckAuthentication (QStandardItem *rootItem,
			Util::SvcAuth::VkAuthManager *authMgr, const ICoreProxy_ptr&);

	std::optional<Media::AudioInfo> TrackMap2Info (const QVariantMap& trackMap);
	QString TrackMap2RadioId (const QVariantMap& trackMap);
}
}
