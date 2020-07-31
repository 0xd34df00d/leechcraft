/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2015  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

namespace LC
{
namespace NetStoreManager
{
	class IStorageAccount;

	class DownManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
	public:
		DownManager (ICoreProxy_ptr proxy, QObject *parent = 0);
	private:
		void SendEntity (const Entity& e);
		void DelegateEntity (const Entity& e, const QString& targetPath, bool openAfterDownload);
	public slots:
		void handleDownloadRequest (const QUrl& url,
			const QString& filePath, TaskParameters tp, bool open);
	};
}
}
