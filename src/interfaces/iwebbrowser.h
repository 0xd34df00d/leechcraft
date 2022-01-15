/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
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
class Q_DECL_EXPORT IWebWidget
{
public:
	virtual ~IWebWidget () = default;

	/** @brief Loads a given url.
	 *
	 * The url should be UTF8-encoded.
	 *
	 * @param[in] url The url of the resource.
	 */
	virtual void Load (const QUrl& url) = 0;

	/** @brief Sets the contents of the web widget to the specified
	 * html.
	 *
	 * External objects such as stylesheets or images referenced in the
	 * HTML document are located relative to the base.
	 *
	 * @param[in] html The HTML with the new contents.
	 * @param[in] base Base address for resolution of external elements.
	 */
	virtual void SetHtml (const QString& html, const QUrl& base = {}) = 0;

	/** @brief Sets whether the navigation bar of the widget (where the
	 * address bar and reload/back/forward/etc buttons are) is visible.
	 *
	 * If the widget doesn't have such bar this function does nothing.
	 *
	 * @param[in] visible Whether to show or hide the navbar.
	 */
	virtual void SetNavBarVisible (bool visible) = 0;

	/** @brief Shows or hides every other panel in the browser but navbar.
	 *
	 * If the IWebBrowser implementation has additional panels,
	 * toolbars, sidebars and similar stuff, it should be set visible
	 * according to the visible parameter.
	 *
	 * @param[in] visible Whether additional stuff should be visible.
	 */
	virtual void SetEverythingElseVisible (bool visible) = 0;

	/** @brief Returns the IWebWidget as a QWidget.
	 *
	 * @return A widget corresponding to this IWebWidget.
	 */
	virtual QWidget* GetQWidget () = 0;

	/** @brief Emitted when the URL rendered by the browser changes to @url@.
	 *
	 * @param[out] url The new URL that the browser renders.
	 */
	virtual void urlChanged (const QUrl&) = 0;
};

class QWebView;

/** @brief Base class for plugins that provide a web browser.
 */
class Q_DECL_EXPORT IWebBrowser
{
public:
	virtual ~IWebBrowser () = default;

	/** @brief Returns the IWebWidget for use in another modules of
	 * LeechCraft.
	 *
	 * @return The IWebWidget.
	 */
	virtual std::unique_ptr<IWebWidget> CreateWidget () const = 0;
};

Q_DECLARE_INTERFACE (IWebWidget, "org.Deviant.LeechCraft.IWebWidget/1.0")
Q_DECLARE_INTERFACE (IWebBrowser, "org.Deviant.LeechCraft.IWebBrowser/1.0")
