/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customwebpage.h"
#include <QTimer>
#include <interfaces/poshuku/iproxyobject.h>
#include "customwebview.h"

namespace LC::Poshuku::WebEngineView
{
	CustomWebPage::CustomWebPage (const ICoreProxy_ptr& proxy, IProxyObject *poshukuProxy, QWidget *parent)
	: QWebEnginePage { parent }
	, Proxy_ { proxy }
	, PoshukuProxy_ { poshukuProxy }
	, LinkOpenModifier_ { poshukuProxy->GetLinkOpenModifier () }
	{
		poshukuProxy->RegisterHookable (this);

		QTimer::singleShot (0, this,
				[this]
				{
					LinkOpenModifier_->InstallOn (view ());
					for (const auto child : view ()->findChildren<QWidget*> ())
						LinkOpenModifier_->InstallOn (child);
				});
	}

	bool CustomWebPage::acceptNavigationRequest (const QUrl& url,
			NavigationType type, bool isMainFrame)
	{
		if (type == NavigationTypeLinkClicked)
		{
			const auto suggestion = LinkOpenModifier_->GetOpenBehaviourSuggestion ();

			LinkOpenModifier_->ResetSuggestionState ();

			if (suggestion.NewTab_)
			{
				auto view = std::make_shared<CustomWebView> (Proxy_, PoshukuProxy_);
				emit webViewCreated (view, suggestion.Invert_);

				view->Load (url, {});

				return false;
			}
		}

		return QWebEnginePage::acceptNavigationRequest (url, type, isMainFrame);
	}
}
