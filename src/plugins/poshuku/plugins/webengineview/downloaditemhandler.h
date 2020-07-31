/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>

class QWebEngineProfile;
class QWebEngineDownloadItem;

namespace LC::Poshuku::WebEngineView
{
	class DownloadItemHandler : public QObject
	{
		const ICoreProxy_ptr Proxy_;
	public:
		DownloadItemHandler (const ICoreProxy_ptr&, QWebEngineProfile*, QObject* = nullptr);
	private:
		void HandleDownloadItem (QWebEngineDownloadItem*);
	};
}
