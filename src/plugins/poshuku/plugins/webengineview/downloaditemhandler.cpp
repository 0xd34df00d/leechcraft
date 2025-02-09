/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downloaditemhandler.h"
#include <QWebEngineProfile>
#if QT_VERSION_MAJOR >= 6
#include <QWebEngineDownloadRequest>
#else
#include <QWebEngineDownloadItem>
#endif
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Poshuku::WebEngineView
{
	DownloadItemHandler::DownloadItemHandler (QWebEngineProfile *prof, QObject *parent)
	: QObject { parent }
	{
		connect (prof,
				&QWebEngineProfile::downloadRequested,
				this,
				[] (auto item)
				{
					item->cancel ();

					auto e = Util::MakeEntity (item->url (),
							{},
							FromUserInitiated | OnlyDownload,
							item->mimeType ());
					auto em = GetProxyHolder ()->GetEntityManager ();
					em->HandleEntity (e);
				});
	}
}
