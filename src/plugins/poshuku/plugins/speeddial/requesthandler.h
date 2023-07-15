/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "interfaces/poshuku/iinternalschemehandler.h"

class QUrlQuery;

namespace LC::Poshuku
{
	class IProxyObject;
}

namespace LC::Poshuku::SpeedDial
{
	extern const QString SpeedDialHost;
	extern const QString SpeedDialUrl;

	using TopList_t = QList<QPair<QUrl, QString>>;

	class CustomSitesManager;
	class ImageCache;

	struct RootPageDeps
	{
		CustomSitesManager& CustomSites_;
		IProxyObject& PoshukuProxy_;
		ImageCache& ImageCache_;
	};

	IInternalSchemeHandler::HandleResult HandleRequest (const QString& path, const QUrlQuery& query, const RootPageDeps& deps);
}
