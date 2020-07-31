/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "interceptadaptor.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebFrame>
#include <util/sll/visitor.h>
#include <util/network/customnetworkreply.h>
#include "customwebview.h"
#include "customwebpage.h"

Q_DECLARE_METATYPE (QNetworkReply*)

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	void InterceptAdaptor::AddInterceptor (const IInterceptableRequests::Interceptor_t& interceptor)
	{
		Interceptors_ << interceptor;
	}

	namespace
	{
		IInterceptableRequests::ResourceType DeriveResourceType (const QNetworkRequest& req)
		{
			auto acceptList = req.rawHeader ("Accept").split (',');
			for (auto& item : acceptList)
			{
				const int pos = item.indexOf (';');
				if (pos > 0)
					item = item.left (pos);
			}
			acceptList.removeAll ("*/*");

			if (acceptList.isEmpty ())
				return IInterceptableRequests::ResourceType::Unknown;

			auto find = [&acceptList] (const auto& f)
			{
				return std::any_of (acceptList.begin (), acceptList.end (), f);
			};

			if (find ([] (const QByteArray& arr) { return arr.startsWith ("image/"); }))
				return IInterceptableRequests::ResourceType::Image;
			if (find ([] (const QByteArray& arr) { return arr == "text/html" || arr == "application/xhtml+xml" || arr == "application/xml"; }))
				return IInterceptableRequests::ResourceType::SubFrame;
			if (find ([] (const QByteArray& arr) { return arr == "text/css"; }))
				return IInterceptableRequests::ResourceType::Stylesheet;

			const auto& path = req.url ().path ();
			if (path.endsWith ("png") || path.endsWith ("jpg") || path.endsWith ("gif"))
				return IInterceptableRequests::ResourceType::Image;
			if (path.endsWith ("js"))
				return IInterceptableRequests::ResourceType::Script;

			return IInterceptableRequests::ResourceType::Unknown;
		}
	}

	void InterceptAdaptor::HandleNAM (const IHookProxy_ptr& hook,
			QNetworkAccessManager*,
			QNetworkAccessManager::Operation*,
			QIODevice**)
	{
		auto req = hook->GetValue ("request").value<QNetworkRequest> ();
		if (!req.originatingObject ())
			return;

		const auto& reqUrl = req.url ();

		if (reqUrl.scheme () == "data")
			return;

		const auto frame = qobject_cast<QWebFrame*> (req.originatingObject ());
		const auto page = frame ? frame->page () : nullptr;

		if (!page)
			return;

		if (frame == page->mainFrame () &&
				frame->requestedUrl () == reqUrl)
			return;

		if (auto cwp = qobject_cast<CustomWebPage*> (page);
			cwp && reqUrl == cwp->GetLoadingURL ())
			return;

		const auto view = qobject_cast<CustomWebView*> (page->view ());
		IInterceptableRequests::RequestInfo info
		{
			reqUrl,
			page->mainFrame ()->url (),
			IInterceptableRequests::NavigationType::Unknown,
			DeriveResourceType (req),
			view ? std::optional<IWebView*> { view } : std::optional<IWebView*> {}
		};

		bool beenRedirected = false;

		for (const auto& interceptor : Interceptors_)
		{
			const auto shouldBlock = Util::Visit (interceptor (info),
					[] (IInterceptableRequests::Allow) { return false; },
					[&] (const IInterceptableRequests::Redirect& r)
					{
						req.setUrl (r.NewUrl_);
						info.RequestUrl_ = r.NewUrl_;
						beenRedirected = true;
						return false;
					},
					[] (IInterceptableRequests::Block) { return true; });

			if (shouldBlock)
			{
				Reject (hook, frame, reqUrl);
				return;
			}
		}

		if (beenRedirected)
			hook->SetValue ("request", QVariant::fromValue (req));
	}

	void InterceptAdaptor::Reject (const IHookProxy_ptr& hook,
			QWebFrame *frame, const QUrl& reqUrl)
	{
		hook->CancelDefault ();

		qDebug () << "rejecting" << frame << reqUrl;

		const auto result = new Util::CustomNetworkReply { reqUrl, frame };
		result->SetContent (QObject::tr ("Blocked"));
		result->setError (QNetworkReply::ContentAccessDenied,
				QObject::tr ("Blocked: %1")
						.arg (reqUrl.toString ()));
		hook->SetReturnValue (QVariant::fromValue<QNetworkReply*> (result));
	}
}
}
}
