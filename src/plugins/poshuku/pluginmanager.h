/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <memory>
#include <QDateTime>
#include <QNetworkRequest>
#include <util/xpc/basehookinterconnector.h>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/poshuku/poshukutypes.h"
#include "interfaces/poshuku/iwebview.h"

class QContextMenuEvent;
class QMenu;

namespace LC
{
namespace Poshuku
{
	class ProxyObject;
	class Core;

	class IWebView;

	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT

		std::shared_ptr<ProxyObject> ProxyObject_;
	public:
		PluginManager (QObject* = nullptr);

		void AddPlugin (QObject*) override;
	signals:
		/** @brief Called when an entry is just added to the favorites.
		 *
		 * This hook is emitted just after an entry with given title,
		 * url and tags is added to favorites. At this point, canceling
		 * the default handler or trying to change other parameters
		 * doesn't make sense. Refer to hookAddToFavoritesRequested()
		 * if you need to modify these.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param title The title of the item just added.
		 * @param url The URL of the item just added.
		 * @param tags The list of tags of the item added.
		 *
		 * @sa hookAddToFavoritesRequested().
		 */
		void hookAddedToFavorites (LC::IHookProxy_ptr proxy,
				QString title, QString url, QStringList tags);

		/** @brief Called when an entry is going to be added to history.
		 *
		 * If the proxy is canceled, no addition takes place at all,
		 * otherwise, the return value from the proxy is considered to
		 * be a list of QVariants. First element (if any) is converted
		 * to string and overrides the history entry title, second one
		 * (if any) is converted to string as well and overrides url,
		 * and third element (if any) is converted to QDateTime and
		 * overrides the date.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param title The title of the item that's going to be added
		 * to history.
		 * @param url The URL of the item.
		 * @param date Datetime of visit (usually current one).
		 */
		void hookAddingToHistory (LC::IHookProxy_ptr proxy,
				QString title, QString url, QDateTime date);

		/** @brief Called when item addition to favorites is requested.
		 *
		 * If the default handler is canceled, the item won't be added
		 * to the favorites. Otherwise, title and url may be overridden
		 * inside the hook by calling IHookProxy::SetValue with names
		 * "title" and "url" respectively, both of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param title The title of the item to be added.
		 * @param url The URL of the item to be added.
		 *
		 * @sa hookAddedToFavorites().
		 */
		void hookAddToFavoritesRequested (LC::IHookProxy_ptr proxy,
				QString title, QString url);

		/** @brief Called when a new browser widget is created and
		 * initialized.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget itself.
		 */
		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);

		/** @brief Called whenever the icon of the page changes.
		 *
		 * The hook may cancel the default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The widget with the browser.
		 */
		void hookIconChanged (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);

		/** @brief Called whenever an icon is requested for url.
		 *
		 * This hook may be used to override an icon for some URLs, for
		 * example. To do this, the hook should cancel the default
		 * implementation, and set the return value of the proxy to
		 * the corresponding QIcon. So, the return value of the proxy
		 * is converted to QIcon if the hook is canceled.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param url The URL for which the icon is requested.
		 */
		void hookIconRequested (LC::IHookProxy_ptr proxy,
				const QUrl& url);

		/** @brief Called when the given page load progress changes.
		 *
		 * The hook may cancel the default implementation, which updates
		 * the load progress indicator. Alternatively, the hook may
		 * change the reported load progress percentage by using
		 * IHookProxy::SetValue() with the key "progress" and the value
		 * of type int.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page whose load progress changed.
		 * @param browserWidget The widget with the page.
		 * @param progress The load progress, in percents.
		 */
		void hookLoadProgress (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				int progress);

		/** @brief Called when the "More" menu begins filling.
		 *
		 * This hook is called when the "More" menu begins filling for
		 * the given web view in the given browserWidget. The hook may
		 * add new items to the menu, for example. Please note that the
		 * menu would be empty before calling the hook, but if another
		 * hook added items to it before your one, your hook would get
		 * a menu with some items already existent.
		 *
		 * Typically, this hook is called whenever a new browser widget
		 * is constructed, and only once during the lifetime of the
		 * widget. The menu won't change during that lifetime, so
		 * you may safely store it in case you need.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param menu The menu that's going to be filled.
		 * @param browserWidget The corresponding browser widget.
		 *
		 * @sa hookMoreMenuFillEnd()
		 */
		void hookMoreMenuFillBegin (LC::IHookProxy_ptr proxy,
				QMenu *menu,
				QObject *browserWidget);

		/** @brief Called when the "More" menu ends filling.
		 *
		 * This method is analogous to hookMoreMenuFillBegin(), but,
		 * instead, it is called after the default implementation has
		 * finished filling the menu.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param menu The menu that's finishing being filled.
		 * @param browserWidget The corresponding browser widget.
		 *
		 * @sa hookMoreMenuFillBegin()
		 */
		void hookMoreMenuFillEnd (LC::IHookProxy_ptr proxy,
				QMenu *menu,
				QObject *browserWidget);

		/** @brief Called when a page finishes loading and user
		 * notification is being prepared.
		 *
		 * The hook may only override the value of the ok parameter by
		 * IHookProxy::SetValue() with name "ok" and value of type bool.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param view The QWebView whose contents finished loading.
		 * @param browserWidget The browser widget containing the view.
		 * @param ok Whether the page finished loading successfully.
		 * @param notifyWhenFinished Whether user chose to be notified
		 * about finished page loads.
		 * @param own Whether the browserWidget is exported to another
		 * plugin.
		 * @param htmlMode Whether the contents of the browserWidget
		 * were obtained by setting the HTML contents directly instead
		 * of loading them from an URL.
		 */
		void hookNotifyLoadFinished (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				bool ok,
				bool notifyWhenFinished,
				bool own,
				bool htmlMode);

		/** @brief Called when session restore is scheduled.
		 *
		 * The urls contains the list of URLs that were chosen to be
		 * restored. If the hook cancels the default handler, then no
		 * URLs may be restored. The hook may choose to open other URLs
		 * after a short amount of time to simulate session restore,
		 * though.
		 *
		 * @note This hook is called before any actual session restore
		 * takes place.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param urls The list of URLs originally scheduled to be
		 * restored.
		 */
		void hookSessionRestoreScheduled (LC::IHookProxy_ptr proxy,
				const QList<QUrl>& urls);

		/** @brief This hook is called when the given URL should be set.
		 *
		 * For example, this hook is called when the browser reacts to
		 * the user input, or some other plugin in LeechCraft wants this
		 * browser widget to navigate to the given URL.
		 *
		 * The hook may cancel the default implementation, or,
		 * alternatively, it may override the URL by calling
		 * IHookProxy::SetValue() with the name "url" and value of type
		 * QUrl.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The widget whose URL is to be set.
		 * @param url The original URL.
		 */
		void hookSetURL (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QUrl url);

		/** @brief Called when the given status bar message is to be
		 * displayed for the given browserWidget.
		 *
		 * The hook may cancel the default implementation, or,
		 * alternatively, override the message to be shown by calling
		 * IHookProxy::SetValue() with the name "message" and value of
		 * type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget for which the message
		 * is to be displayed.
		 * @param message The original message to be displayed.
		 */
		void hookStatusBarMessage (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString message);

		/** @brief Called when tabbar context menu is requested.
		 *
		 * The hook may choose to add new actions to the context menu,
		 * in this case it should use the IHookProxy::SetValue() with
		 * the name "actions", and a list of actions (QList<QObject*>)
		 * should be inserted there. Value with the name "endActions"
		 * may also be used, and the corresponding actions would be
		 * appended to the end of the menu.
		 *
		 * Please note that other hooks may have already inserted hooks
		 * into the list, so a well-written hook should first extract
		 * the list of already-added actions and append new ones to that
		 * list.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget for which tabbar
		 * actions are requested.
		 */
		void hookTabBarContextMenuActions (LC::IHookProxy_ptr proxy,
				const QObject *browserWidget) const;

		void hookTabAdded (LC::IHookProxy_ptr proxy,
				QObject *browserWidget,
				const QUrl& url);

		void hookTabRemoveRequested (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookUpdateLogicalPath (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookURLCompletionNewStringRequested (LC::IHookProxy_ptr proxy,
				QObject *model,
				const QString& string,
				int historyItems);
		void hookURLEditReturnPressed (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookUserAgentForUrlRequested (LC::IHookProxy_ptr proxy,
				const QUrl& url);

		void hookAcceptNavigationRequest (LC::IHookProxy_ptr proxy,
				const QUrl& request,
				IWebView *view,
				IWebView::NavigationType type,
				bool isMainFrame);

		/** @brief Called when context menu for the view is requested.
		 *
		 * This hook is called when building the context menu after user
		 * has requested it.
		 *
		 * The building has several different stages in which actions of
		 * different semantics are added to the menu, so the hook may
		 * choose the most appropriate build stage for injecting its
		 * actions, for example. Consecutively, the hook is called
		 * multiple times for the given menu.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param view The web view for which the context menu is
		 * requested.
		 * @param hitTestResult The structure describing the context menu
		 * mouse hit.
		 * @param menu The menu being built.
		 * @param menuBuildStage The stage of the menu being built.
		 */
		void hookWebViewContextMenu (LC::IHookProxy_ptr proxy,
				LC::Poshuku::IWebView *view,
				const LC::Poshuku::ContextMenuInfo& hitTestResult,
				QMenu *menu,
				WebViewCtxMenuStage menuBuildStage);
	};
}
}
