/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utils.h"
#include <QUrl>
#include <QIcon>
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/sys/resourceloader.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LC
{
namespace NetStoreManager
{
namespace Utils
{
	QStringList ScanDir (QDir::Filters filter, const QString& path, bool recursive)
	{
		QDir baseDir (path);
		QStringList paths;
		for (const auto& entry : baseDir.entryInfoList (filter))
		{
			paths << entry.absoluteFilePath ();
			if (recursive &&
					entry.isDir ())
				paths << ScanDir (filter, entry.absoluteFilePath (), recursive);
		}
		return paths;
	}

	bool RemoveDirectoryContent (const QString& dirPath)
	{
		bool result = true;
		QDir dir (dirPath);

		if (dir.exists (dirPath))
		{
			for (const auto& info : dir.entryInfoList (QDir::NoDotAndDotDot | QDir::AllEntries))
			{
				if (info.isDir ())
					result = RemoveDirectoryContent (info.absoluteFilePath ());
				else
					result = QFile::remove (info.absoluteFilePath ());

				if (!result)
					return result;
			}

			result = dir.rmdir (dirPath);
		}

		return result;
	}

	std::function<void (ISupportFileListings::RequestUrlResult_t)> HandleRequestFileUrlResult (IEntityManager *entityMgr,
			const QString& errorText,
			const std::function<void (QUrl)>& urlHandler)
	{
		return Util::Visitor
		{
			std::move (urlHandler),
			Util::Visitor
			{
				[] (const ISupportFileListings::UserCancelled&) {},
				[] (const ISupportFileListings::InvalidItem&)
				{
					qWarning () << Q_FUNC_INFO
							<< "invalid item";
				},
				[=] (const QString& errStr)
				{
					const auto& e = Util::MakeNotification ("NetStoreManager",
							errorText + " " + errStr,
							Priority::Critical);
					entityMgr->HandleEntity (e);
				}
			}
		};
	}

	QIcon GetStorageIcon (IStoragePlugin *plugin)
	{
		static const auto loader = []
		{
			auto loader = std::make_unique<Util::ResourceLoader> ("netstoremanager/services");
			loader->AddGlobalPrefix ();
			loader->AddLocalPrefix ();
			return loader;
		} ();
		return loader->LoadPixmap (plugin->GetStorageIconName ());
	}
}
}
}
