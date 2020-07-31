/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkAccessManager>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/iinterceptablerequests.h>

class QWebFrame;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class InterceptAdaptor
	{
		QList<IInterceptableRequests::Interceptor_t> Interceptors_;
	public:
		void AddInterceptor (const IInterceptableRequests::Interceptor_t&);

		void HandleNAM (const IHookProxy_ptr&,
				QNetworkAccessManager*,
				QNetworkAccessManager::Operation*,
				QIODevice**);
	private:
		void Reject (const IHookProxy_ptr&, QWebFrame*, const QUrl&);
	};
}
}
}
