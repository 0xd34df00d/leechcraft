/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDateTime>
#include <QFuture>
#include <interfaces/azoth/ihaveserverhistory.h>

class QNetworkReply;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;

	class ServerMessagesSyncer : public QObject
	{
		const QDateTime Since_;
		VkAccount * const Acc_;

		int Offset_ = 0;

		QFutureInterface<IHaveServerHistory::DatedFetchResult_t> Iface_;

		IHaveServerHistory::MessagesSyncMap_t Messages_;
	public:
		ServerMessagesSyncer (const QDateTime&, VkAccount*, QObject* = nullptr);

		QFuture<IHaveServerHistory::DatedFetchResult_t> GetFuture ();
	private:
		void Request ();
		void HandleFinished (QNetworkReply*);

		void HandleDone ();
		void ReportError (const QString&);
	};
}
}
}
