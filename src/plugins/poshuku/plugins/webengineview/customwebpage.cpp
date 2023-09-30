/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customwebpage.h"
#include <QTimer>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "customwebview.h"

namespace LC::Poshuku::WebEngineView
{
	CustomWebPage::CustomWebPage (IProxyObject *poshukuProxy, CustomWebView *parent)
	: QWebEnginePage { parent }
	, View_ { parent }
	, PoshukuProxy_ { poshukuProxy }
	, LinkOpenModifier_ { poshukuProxy->GetLinkOpenModifier () }
	{
		poshukuProxy->RegisterHookable (this);

		LinkOpenModifier_->InstallOn (parent);
		for (const auto child : parent->findChildren<QWidget*> ())
			LinkOpenModifier_->InstallOn (child);
	}

	namespace
	{
		IWebView::NavigationType ConvertType (QWebEnginePage::NavigationType type)
		{
#define LC_TYPE(x) \
			case QWebEnginePage::NavigationType##x: \
				return IWebView::NavigationType::x;

			switch (type)
			{
			LC_TYPE (LinkClicked)
			LC_TYPE (Typed)
			LC_TYPE (FormSubmitted)
			LC_TYPE (BackForward)
			LC_TYPE (Reload)
			LC_TYPE (Redirect)
			LC_TYPE (Other)
			}
#undef LC_TYPE

			qWarning () << "unknown type"
					<< type;

			return IWebView::NavigationType::Other;
		}
	}

	bool CustomWebPage::acceptNavigationRequest (const QUrl& url,
			NavigationType type, bool isMainFrame)
	{
		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAcceptNavigationRequest (proxy, url, View_, ConvertType (type), isMainFrame);

		if (proxy->IsCancelled ())
			return false;

		if (type == NavigationTypeLinkClicked)
		{
			const auto suggestion = LinkOpenModifier_->GetOpenBehaviourSuggestion ();

			LinkOpenModifier_->ResetSuggestionState ();

			if (suggestion.NewTab_)
			{
				using namespace NewWebViewBehavior;

				auto view = std::make_shared<CustomWebView> (PoshukuProxy_);
				emit webViewCreated (view, suggestion.IsBackground_ ? Background : None);

				view->Load (url, {});

				return false;
			}
		}

		return QWebEnginePage::acceptNavigationRequest (url, type, isMainFrame);
	}
}
