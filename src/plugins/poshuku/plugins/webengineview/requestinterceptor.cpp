/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requestinterceptor.h"
#include <QWebEngineUrlRequestInfo>
#include <QtDebug>
#include <util/sll/visitor.h>
#include "customwebview.h"

namespace LC::Poshuku::WebEngineView
{
	namespace
	{
		IInterceptableRequests::ResourceType ConvertResourceType (QWebEngineUrlRequestInfo::ResourceType type)
		{
#define HANDLE(x)	case QWebEngineUrlRequestInfo::ResourceType##x: \
						return IInterceptableRequests::ResourceType::x;

			switch (type)
			{
			HANDLE (MainFrame)
			HANDLE (SubFrame)
			HANDLE (Stylesheet)
			HANDLE (Script)
			HANDLE (Image)
			HANDLE (FontResource)
			HANDLE (SubResource)
			HANDLE (Object)
			HANDLE (Media)
			HANDLE (Worker)
			HANDLE (SharedWorker)
			HANDLE (Prefetch)
			HANDLE (Favicon)
			HANDLE (Xhr)
			HANDLE (Ping)
			HANDLE (ServiceWorker)
			HANDLE (CspReport)
			HANDLE (PluginResource)
			HANDLE (Unknown)
			case QWebEngineUrlRequestInfo::ResourceTypeLast:
				qWarning () << "got ResourceTypeLast";
				return IInterceptableRequests::ResourceType::Unknown;
			}
#undef HANDLE

			return IInterceptableRequests::ResourceType::Unknown;
		}
	}

	void RequestInterceptor::interceptRequest (QWebEngineUrlRequestInfo& info)
	{
		const auto& pageUrl = info.firstPartyUrl ();
		const auto page = [&] () -> std::optional<IWebView*>
		{
			for (const auto view : Views_)
				if (pageUrl == view->GetUrl ())
					return view;
			return {};
		} ();
		IInterceptableRequests::RequestInfo convertedInfo
		{
			info.requestUrl (),
			pageUrl,
			IInterceptableRequests::NavigationType::Unknown,
			ConvertResourceType (info.resourceType ()),
			page
		};

		for (const auto& interceptor : Interceptors_)
		{
			const auto shouldBlock = Util::Visit (interceptor (convertedInfo),
					[] (IInterceptableRequests::Allow) { return false; },
					[&] (const IInterceptableRequests::Redirect& r)
					{
						info.redirect (r.NewUrl_);
						return false;
					},
					[] (IInterceptableRequests::Block) { return true; });

			if (shouldBlock)
				info.block (true);
		}
	}

	void RequestInterceptor::Add (const IInterceptableRequests::Interceptor_t& interceptor)
	{
		Interceptors_ << interceptor;
	}

	void RequestInterceptor::RegisterView (CustomWebView *view)
	{
		Views_ << view;

		connect (view,
				&QObject::destroyed,
				this,
				[this, view] { Views_.removeOne (view); });
	}
}
