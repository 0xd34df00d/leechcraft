/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webviewsslwatcherhandler.h"
#include <QAction>
#include <interfaces/core/iiconthememanager.h>
#include "webpagesslwatcher.h"
#include "customwebview.h"
#include "sslstatedialog.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebViewSslWatcherHandler::WebViewSslWatcherHandler (CustomWebView *view, IIconThemeManager *itm)
	: QObject { view }
	, View_ { view }
	, SslWatcher_ { new WebPageSslWatcher { view } }
	, SslStateAction_ { new QAction { this } }
	, ITM_ { itm }
	{
		connect (SslWatcher_,
				SIGNAL (sslStateChanged (WebPageSslWatcher*)),
				this,
				SLOT (handleSslState ()));

		connect (SslStateAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (showSslDialog ()));
	}

	QAction* WebViewSslWatcherHandler::GetStateAction () const
	{
		return SslStateAction_;
	}

	void WebViewSslWatcherHandler::handleSslState ()
	{
		QString iconName;
		QString title;
		switch (SslWatcher_->GetPageState ())
		{
		case WebPageSslWatcher::State::NoSsl:
			SslStateAction_->setEnabled (false);
			return;
		case WebPageSslWatcher::State::SslErrors:
			iconName = "security-low";
			title = tr ("Some SSL errors where encountered.");
			break;
		case WebPageSslWatcher::State::UnencryptedElems:
			iconName = "security-medium";
			title = tr ("Some elements were loaded via unencrypted connection.");
			break;
		case WebPageSslWatcher::State::FullSsl:
			iconName = "security-high";
			title = tr ("Everything is secure!");
			break;
		}

		SslStateAction_->setIcon (ITM_->GetIcon (iconName));
		SslStateAction_->setText (title);
		SslStateAction_->setEnabled (true);
	}

	void WebViewSslWatcherHandler::showSslDialog ()
	{
		const auto dia = new SslStateDialog { SslWatcher_, ITM_ };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
}
