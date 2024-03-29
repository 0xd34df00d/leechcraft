/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWebEnginePage>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/iwebview.h>
#include <interfaces/poshuku/ilinkopenmodifier.h>
#include <interfaces/poshuku/iwebviewprovider.h>

namespace LC::Poshuku
{
class IProxyObject;

namespace WebEngineView
{
	class CustomWebView;

	class CustomWebPage : public QWebEnginePage
	{
		Q_OBJECT

		CustomWebView * const View_;
		IProxyObject * const PoshukuProxy_;
		const ILinkOpenModifier_ptr LinkOpenModifier_;
	public:
		CustomWebPage (IProxyObject*, CustomWebView*);
	protected:
		bool acceptNavigationRequest (const QUrl&, NavigationType, bool) override;
		QWebEnginePage* createWindow (WebWindowType) override;
	signals:
		void webViewCreated (const std::shared_ptr<CustomWebView>&, NewWebViewBehavior::Enum);

		void hookAcceptNavigationRequest (LC::IHookProxy_ptr proxy,
				const QUrl& request,
				IWebView *view,
				IWebView::NavigationType type,
				bool isMainFrame);
	};
}
}
