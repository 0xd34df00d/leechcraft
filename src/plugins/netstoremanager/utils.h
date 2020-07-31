/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>
#include <QDir>
#include "interfaces/netstoremanager/isupportfilelistings.h"

class IEntityManager;

namespace LC
{
namespace NetStoreManager
{
class IStoragePlugin;

namespace Utils
{
	QStringList ScanDir (QDir::Filters filter, const QString& path, bool recursive = false);
	bool RemoveDirectoryContent (const QString& dirPath);

	std::function<void (ISupportFileListings::RequestUrlResult_t)> HandleRequestFileUrlResult (IEntityManager *entityMgr,
			const QString& errorText,
			const std::function<void (QUrl)>& urlHandler);

	QIcon GetStorageIcon (IStoragePlugin*);
}
}
}
