/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downloaditemhandler.h"
#include <QWebEngineProfile>
#include <QWebEngineDownloadItem>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Poshuku::WebEngineView
{
	DownloadItemHandler::DownloadItemHandler (const ICoreProxy_ptr& proxy, QWebEngineProfile *prof, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	{
		connect (prof,
				&QWebEngineProfile::downloadRequested,
				this,
				&DownloadItemHandler::HandleDownloadItem);
	}

	void DownloadItemHandler::HandleDownloadItem (QWebEngineDownloadItem *item)
	{
		item->cancel ();

		auto e = Util::MakeEntity (item->url (),
				{},
				FromUserInitiated,
				item->mimeType ());
		e.Additional_ ["IgnorePlugins"] = "org.LeechCraft.Poshuku";
		e.Additional_ ["AllowedSemantics"] = QStringList { "fetch", "save" };

		auto em = Proxy_->GetEntityManager ();
		em->HandleEntity (e);
	}
}
