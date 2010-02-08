/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef INTERFACES_IWEBBROWSER_H
#define INTERFACES_IWEBBROWSER_H
#include <QString>
#include <QWidget>
#include <QUrl>
#include <QtPlugin>

/** @brief Common interface for a web widget.
 *
 * A widget that is capable of showing/rendering web pages should
 * implement this interface in order to communicate with other modules
 * of LeechCraft.
 */
class IWebWidget
{
public:
	virtual ~IWebWidget () {}

	/** @brief Loads a given url.
	 *
	 * The url should be UTF8-encoded.
	 *
	 * @param[in] url The url of the resource.
	 */
	virtual void Load (const QString& url) = 0;

	/** @brief Sets the contents of the web widget to the specified
	 * html.
	 *
	 * External objects such as stylesheets or images referenced in the
	 * HTML document are located relative to the base.
	 *
	 * @param[in] html The HTML with the new contents.
	 * @param[in] base Base address for resolution of external elements.
	 */
	virtual void SetHtml (const QString& html,
			const QUrl& base = QUrl ()) = 0;

	/** @brief Sets whether the navigation bar of the widget (where the
	 * address bar and reload/back/forward/etc buttons are) is visible.
	 *
	 * If the widget doesn't have such bar this function does nothing.
	 *
	 * @param[in] visible Whether to show or hide the navbar.
	 */
	virtual void SetNavBarVisible (bool visible) = 0;

	/** @brief Returns the IWebWidget as a QWidget.
	 *
	 * @return A widget corresponding to this IWebWidget.
	 */
	virtual QWidget* Widget () = 0;
};

class QWebView;

/** @brief Base class for plugins that provide a web browser.
 */
class IWebBrowser
{
public:
	/** @brief Opens the url in the web browser itself.
	 * 
	 * @param[in] url The URL to open.
	 */
	virtual void Open (const QString& url) = 0;

	/** @brief Returns the IWebWidget for use in another modules of
	 * LeechCraft.
	 *
	 * The ownership transfers to the caller.
	 *
	 * @return The IWebWidget.
	 */
	virtual IWebWidget* GetWidget () const = 0;

	virtual QWebView* CreateWindow () = 0;

	virtual ~IWebBrowser () {}
};

Q_DECLARE_INTERFACE (IWebWidget, "org.Deviant.LeechCraft.IWebWidget/1.0");
Q_DECLARE_INTERFACE (IWebBrowser, "org.Deviant.LeechCraft.IWebBrowser/1.0");

#endif

