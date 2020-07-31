/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEngineUrlRequestInterceptor>
#include <interfaces/poshuku/iinterceptablerequests.h>

namespace LC::Poshuku::WebEngineView
{
	class CustomWebView;

	class RequestInterceptor : public QWebEngineUrlRequestInterceptor
	{
		QList<IInterceptableRequests::Interceptor_t> Interceptors_;
		QList<CustomWebView*> Views_;
	public:
		void interceptRequest (QWebEngineUrlRequestInfo&) override;

		void Add (const IInterceptableRequests::Interceptor_t&);
		void RegisterView (CustomWebView*);
	};
}
