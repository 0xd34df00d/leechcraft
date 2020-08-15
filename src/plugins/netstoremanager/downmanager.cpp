/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2015  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downmanager.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/netstoremanager/istorageaccount.h>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>

namespace LC
{
namespace NetStoreManager
{
	DownManager::DownManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ (proxy)
	{
	}

	void DownManager::SendEntity (const Entity& e)
	{
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void DownManager::DelegateEntity (const Entity& e, const QString& targetPath,
			bool openAfterDownload)
	{
		auto res = Proxy_->GetEntityManager ()->DelegateEntity (e);
		if (!res)
		{
			auto notif = Util::MakeNotification ("NetStoreManager",
				tr ("Could not find plugin to download %1.")
						.arg ("<em>" + e.Entity_.toString () + "</em>"),
				Priority::Critical);
			SendEntity (notif);
			return;
		}

		Util::Sequence (this, res.DownloadResult_) >>
				Util::Visitor
				{
					[=, this] (IDownload::Success)
					{
						if (openAfterDownload)
							SendEntity (Util::MakeEntity (QUrl::fromLocalFile (targetPath),
									{}, OnlyHandle | FromUserInitiated));
					},
					[] (IDownload::Error) {}
				};
	}

	void DownManager::handleDownloadRequest (const QUrl& url,
			const QString& filePath, TaskParameters tp, bool open)
	{
		const auto& savePath = open ?
				QStandardPaths::writableLocation (QStandardPaths::TempLocation) + "/" + filePath :
				filePath;

		auto e = Util::MakeEntity (url, savePath, tp);
		e.Additional_ ["Filename"] = QFileInfo (filePath).fileName ();
		open ?
			DelegateEntity (e, savePath, open) :
			SendEntity (e);
	}
}
}
