/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef UTIL_SELECTABLEBROWSER_H
#define UTIL_SELECTABLEBROWSER_H
#include <memory>
#include <QWidget>
#include <QTextBrowser>
#include <interfaces/iwebbrowser.h>
#include "guiconfig.h"

namespace LC
{
	namespace Util
	{
		/** @brief A "browser" that shows HTML either via QTextBrowser or
		 * a browser plugin.
		 *
		 * This class is used to display HTML content via some web
		 * browser plugin (like Poshuku) and, if such plugins aren't
		 * available, downgrade to QTextBrowser.
		 *
		 * Currently using this class is generally discouraged. QtWebKit
		 * isn't as new and uncommon as it was a couple of years ago, so
		 * in most cases it's easier to just embed a QWebView.
		 *
		 * @ingroup GuiUtil
		 */
		class UTIL_GUI_API SelectableBrowser : public QWidget
		{
			Q_OBJECT

			bool Internal_;
			std::unique_ptr<QTextBrowser> InternalBrowser_;
			std::unique_ptr<IWebWidget> ExternalBrowser_;
		public:
			/** @brief Constructs the browser with the given parent.
			 *
			 * By default, if no other functions are called, this browser
			 * widget will use QTextBrowser to display content. Call
			 * Construct() and pass a pointer to a web browser plugin to
			 * use it.
			 *
			 * @param[in] parent The parent widget.
			 *
			 * @sa Construct()
			 */
			SelectableBrowser (QWidget *parent = 0);

			/** @brief Initialize the widget with the browser plugin.
			 *
			 * After calling this function the widget will use the
			 * browser plugin to show HTML content.
			 *
			 * If the browser is nullptr, this function does nothing and
			 * the widget still uses the QTextBrowser.
			 *
			 * Any HTML content currently set will be cleared after
			 * calling this function.
			 *
			 * @param[in] browser The browser to use.
			 */
			void Construct (IWebBrowser *browser);

			/** @brief Sets the HTML content to display.
			 *
			 * @param[in] html The HTML content to display.
			 * @param[in] base The base URL which is used to resolve
			 * relative URLs found in the HTML content.
			 */
			void SetHtml (const QString& html, const QUrl& base = QUrl ());

			/** @brief Sets whether navigation bar should be visible.
			 *
			 * The navigation bar typically includes the address edit
			 * button with back/forward/reload buttons. Some plugins may
			 * find it undesirable.
			 *
			 * The default state is unspecified.
			 *
			 * @param[in] visible Whether navigation bar should be
			 * visible.
			 */
			void SetNavBarVisible (bool visible);

			/** @brief Sets whether other UI elements should be visible.
			 *
			 * Sometimes it's not desirable to show, for example, browser
			 * history or bookmarks panes. Use this function to hide
			 * them.
			 *
			 * The default state is unspecified.
			 *
			 * @param[in] visible Whether irrelevant UI elements should be
			 * visible.
			 */
			void SetEverythingElseVisible (bool visible);
		private:
			void PrepareInternal ();
		};
	};
};

#endif

