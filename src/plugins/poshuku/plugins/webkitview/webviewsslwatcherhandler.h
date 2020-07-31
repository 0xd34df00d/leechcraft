/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QAction;
class IIconThemeManager;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class CustomWebView;
	class WebPageSslWatcher;

	class WebViewSslWatcherHandler : public QObject
	{
		Q_OBJECT

		CustomWebView * const View_;

		WebPageSslWatcher * const SslWatcher_;
		QAction * const SslStateAction_;
		IIconThemeManager * const ITM_;
	public:
		WebViewSslWatcherHandler (CustomWebView*, IIconThemeManager*);

		QAction* GetStateAction () const;
	private slots:
		void handleSslState ();
		void showSslDialog ();
	};
}
}
}
