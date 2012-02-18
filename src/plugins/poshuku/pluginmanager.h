/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINMANAGER_H
#define PLUGINS_POSHUKU_PLUGINMANAGER_H
#include <vector>
#include <memory>
#include <QWebPage>
#include <QDateTime>
#include <QNetworkRequest>
#include <util/basehookinterconnector.h>
#include <interfaces/iinfo.h>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/poshukutypes.h"
#include "interfaces/iwebplugin.h"

class QGraphicsWebView;
class QGraphicsSceneContextMenuEvent;

namespace LeechCraft
{
namespace Poshuku
{
	class ProxyObject;
	class Core;

	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT

		std::shared_ptr<ProxyObject> ProxyObject_;
	public:
		PluginManager (QObject* = 0);

		virtual void AddPlugin (QObject*);
	signals:
		/** @brief Called whenever WebKit requests to navigate frame to
		 * the resource specified by request.
		 *
		 * The hook may modify the request, by using the
		 * IHookProxy::SetValue method with the name "request".
		 *
		 * If default handler is canceled, then its return value is
		 * converted to bool and is used as the return value of the
		 * QWebPage::acceptNavigationRequest() method, otherwise
		 * standard LeechCraft handler is used.
		 *
		 * Refer also to the documentation of the
		 * QWebPage::acceptNavigationRequest(), from which this hook is
		 * called.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage from which this request originates.
		 * @param frame The frame to navigate, or null pointer if
		 * navigating to new window is requested.
		 * @param request The original network request.
		 * @param type The navigation type.
		 */
		void hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QNetworkRequest request,
				QWebPage::NavigationType type);

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
		void hookAddedToFavorites (LeechCraft::IHookProxy_ptr proxy,
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
		 * @param browserWidget The BrowserWidget from which this
		 * request originates.
		 */
		void hookAddingToHistory (LeechCraft::IHookProxy_ptr proxy,
				QString title, QString url, QDateTime date, QObject *browserWidget);

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
		void hookAddToFavoritesRequested (LeechCraft::IHookProxy_ptr proxy,
				QString title, QString url);

		/** @brief Called inside QWebPage::chooseFile().
		 *
		 * This hook is called when the web content requests a a file
		 * name. If default handler is canceled, the proxy's return
		 * value is converted to QString and returned from the
		 * QWebPage::chooseFile() method, otherwise default handler is
		 * used. In the latter case, the hook may override the suggested
		 * filename by using IHookProxy::SetValue with the key
		 * "suggested" and value of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page that this request originated from.
		 * @param frame The frame that this request originated from.
		 * @param suggested The suggested filename.
		 */
		void hookChooseFile (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString suggested);

		/** @brief This hook is called whenever page contents change.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page whose contents changed.
		 */
		void hookContentsChanged (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called whenever an HTML object element is encountered.
		 *
		 * This hook is called inside QWebPage::createPlugin().
		 *
		 * If the default handler is canceled, the return value of the
		 * proxy is casted to QObject* and returned from the
		 * QWebPage::createPlugin(), otherwise the default handler is
		 * used. In the latter case, the hook may override the
		 * parameters by using IHookProxy::SetValue() with the following
		 * keys and values of corresponding types:
		 * - "clsid" of type QString
		 * - "url" of type QUrl
		 * - "params" of type QStringList
		 * - "values" of type QStringList
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page on which the element was
		 * encountered.
		 * @param clsid Original class ID.
		 * @param url Original URL of the object.
		 * @param params List of parameters to the plugin.
		 * @param values List of values of the parameters.
		 */
		void hookCreatePlugin (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString clsid,
				QUrl url,
				QStringList params,
				QStringList values);

		/** @brief Called from QWebPage::createWindow().
		 *
		 * This hook is called whenever a new window of the given type
		 * should be created.
		 *
		 * If the default handler is canceled, the return value of the
		 * proxy is casted to QWebPage* and returned from the
		 * QWebPage::createWindow(). If the cast is successful, then
		 * the new window would be created, otherwise (or if a null
		 * pointer is returned) the window won't be created.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page originating the new window request.
		 * @param type The type of the new window.
		 */
		void hookCreateWindow (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::WebWindowType type);

		/** @brief Called from QWebPage::databaseQuotaExceeded().
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page originating the error.
		 * @param sourceFrame The frame with the site trying to store
		 * excessive data.
		 * @param databaseName The name of the database.
		 */
		void hookDatabaseQuotaExceeded (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *sourceFrame,
				QString databaseName);

		/** @brief Called from QWebPage::downloadRequested().
		 *
		 * If the default handler is canceled, nothing is done at all.
		 * Otherwise, the entity with the given request would be emitted
		 * to be downloaded by some other LeechCraft plugin. The hook
		 * may override the request by IHookProxy::SetValue() method
		 * with the name "request" and value of type QNetworkRequest.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page originating the request.
		 * @param request The original download request.
		 */
		void hookDownloadRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QNetworkRequest request);

		/** @brief Called from QWebPage::extension().
		 *
		 * If the default handler is canceled, the return value of the
		 * hook proxy is converted to bool and returned from the
		 * QWebPage::extension() method, otherwise the default handler
		 * is used.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page for which the extension should be
		 * handled.
		 * @param extension The extension.
		 * @param extensionOption The struct with options of the
		 * extension.
		 * @param extensionReturn The struct with return value of the
		 * extension.
		 *
		 * @sa hookSupportsExtension().
		 */
		void hookExtension (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::Extension extension,
				const QWebPage::ExtensionOption* extensionOption,
				QWebPage::ExtensionReturn* extensionReturn);

		/** @brief Called when user wants to find the given text.s
		 *
		 * This hook is called when the user wants to find the given
		 * text on the given browserWidget. The hook may cancel the
		 * default handler or modify the text by calling the
		 * IHookProxy::SetValue() method with the "text" key and a value
		 * of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget originating the text
		 * find request.
		 * @param text The original text to be found by the user.
		 * @param findFlags The text find options.
		 */
		void hookFindText (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString text,
				QWebPage::FindFlags findFlags);

		/** @brief Called whenever a new frame is created.
		 *
		 * This hook is called whenever a new frame is created on the
		 * given page.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page on which the frame is created.
		 * @param frameCreated The newly created frame.
		 */
		void hookFrameCreated (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frameCreated);

		/** @brief Called from QWebPage::geometryChangeRequested().
		 *
		 * This hook is called whenever the document want to change the
		 * page's position and size.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that wants its position and size to be
		 * changed.
		 * @param rect The new position and size rect.
		 */
		void hookGeometryChangeRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QRect rect);

		/** @brief Called whenever the icon of the page changes.
		 *
		 * The hook may cancel the default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page that has its icon changed.
		 * @param browserWidget The widget with the browser.
		 */
		void hookIconChanged (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
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
		void hookIconRequested (LeechCraft::IHookProxy_ptr proxy,
				const QUrl& url);

		/** @brief Called when the frame is laid out for the first time.
		 *
		 * This hook is called whenever the frame is laid out for the
		 * first time during web page load.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the frame being laid out is.
		 * @param frame The frame that's laid out for the first time.
		 */
		void hookInitialLayoutCompleted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame);

		/** @brief Called from QWebPage::javaScriptAlert().
		 *
		 * This hook is called whenever a JS in the given frame wants to
		 * alert() the given message.
		 *
		 * The default implementation may be canceled inside the hook.
		 * Alternatively, the hook may override the message with
		 * IHookProxy::SetValue() with the key "message" and value of
		 * type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the frame is.
		 * @param frame The frame containing the JS that wants to
		 * alert() a message.
		 * @param message The original message text.
		 */
		void hookJavaScriptAlert (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message);

		/** @brief Called from QWebPage::javaScriptConfirm().
		 *
		 * This hook is called whenever a JS program in the given frame
		 * calls confirm() with the given message.
		 *
		 * If the default implementation is canceled from the hook, the
		 * proxy's return value is converted to bool and returned from
		 * the QWebPage::javaScriptConfirm() method. Otherwise, the hook
		 * may override the message with IHookProxy::SetValue() with the
		 * key "message and value of type QString.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param frame The frame containing the JS that wants to
		 * confirm() something.
		 * @param message The original message text.
		 */
		void hookJavaScriptConfirm (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message);

		/** @brief Called from QWebPage::javaScriptConsoleMessage().
		 *
		 * This hook is called whenever a JS program wants to print a
		 * message to the web browser console.
		 *
		 * If the default implementation isn't canceled, the hook may
		 * override the message, line and sourceId using
		 * IHookProxy::SetValue() method with following keys and values,
		 * respectively:
		 * - "message" of type QString
		 * - "line" of type int
		 * - "sourceID" of type QString
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param message The original message text.
		 * @param line The line, if applicable.
		 * @param sourceId The ID of the source, if applicable.
		 */
		void hookJavaScriptConsoleMessage (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString message,
				int line,
				QString sourceId);

		/** @brief Called from QWebPage::javaScriptPrompt().
		 *
		 * This hook is called whenever a JS program in the given frame
		 * wants user to input a string via prompt().
		 *
		 * If the default handler is canceled, the proxy's return value
		 * is converted to bool and returned from the
		 * QWebPage::javaScriptPrompt(), while the value set using
		 * IHookProxy::SetValue() with the key "result" is converted to
		 * QString and is considered to be the result. Otherwise, other
		 * parameters may be overridden in the hook in addition to
		 * "result", respectively:
		 * - "message" of type QString
		 * - "default" of type QString
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page containing the frame.
		 * @param frame The frame containing the JS that called
		 * prompt().
		 * @param message The original message text.
		 * @param defValue The default value suggested by the JS.
		 * @param resultString The result string.
		 */
		void hookJavaScriptPrompt (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString message,
				QString defValue,
				QString resultString);

		/** @brief Called when the global window object of the JS
		 * environment is cleared in the given frame.
		 *
		 * To ensure that objects added to a QWebFrame via the
		 * QWebFrame::addToJavaScriptWindowObject() method are
		 * accessible when loading new URLs, they should be readded
		 * in the corresponding hook.
		 *
		 * If the default handler is canceled, the "window.JSProxy" and
		 * "window.external" objects won't be added to the frame.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param sourcePage The page containing the frame.
		 * @param frame The frame whose window object is cleared.
		 */
		void hookJavaScriptWindowObjectCleared (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *frame);

		/** @brief Called whenever the given link is clicked.
		 * @deprecated
		 *
		 * Seems like this hook is no longer used.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the link is clicked.
		 * @param url The URL of the link.
		 */
		void hookLinkClicked (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QUrl url);

		/** @brief Called whenever a given link is hovered by the mouse.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page where the link is hovered.
		 * @param link The URL of the link.
		 * @param title The HTML link element title, if specified in the
		 * markup.
		 * @param textContext The text within the HTML link element.
		 */
		void hookLinkHovered (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString link,
				QString title,
				QString textContent);

		/** @brief Called when the given page finishes loading.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that has finished loading.
		 * @param result Whether the page loaded successfully.
		 */
		void hookLoadFinished (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				bool result);

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
		void hookLoadProgress (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QObject *browserWidget,
				int progress);

		/** @brief Called when the given page begins loading.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that has started loading.
		 */
		void hookLoadStarted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called when the "More" menu begins filling.
		 *
		 * This hook is called when the "More" menu begins filling for
		 * the given webView in the given browserWidget. The hook may
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
		 * @param webView The QGraphicsWebView the menu is associated with.
		 * @param browserWidget The browser widget with the webView.
		 *
		 * @sa hookMoreMenuFillEnd()
		 */
		void hookMoreMenuFillBegin (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu,
				QGraphicsWebView *webView,
				QObject *browserWidget);

		/** @brief Called when the "More" menu ends filling.
		 *
		 * This method is analogous to hookMoreMenuFillBegin(), but,
		 * instead, it is called after the default implementation has
		 * finished filling the menu.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param menu The menu that's finishing being filled.
		 * @param webView The QGraphicsWebView the menu is associated with.
		 * @param browserWidget The browser widget containing webView.
		 *
		 * @sa hookMoreMenuFillBegin()
		 */
		void hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu,
				QGraphicsWebView *webView,
				QObject *browserWidget);

		/** @brief Called when a page finishes loading and user
		 * notification is being prepared.
		 *
		 * The hook may only override the value of the ok parameter by
		 * IHookProxy::SetValue() with name "ok" and value of type bool.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param view The QGraphicsWebView whose contents finished loading.
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
		void hookNotifyLoadFinished (LeechCraft::IHookProxy_ptr proxy,
				QGraphicsWebView *view,
				QObject *browserWidget,
				bool ok,
				bool notifyWhenFinished,
				bool own,
				bool htmlMode);

		/** @brief Called when the frame is to be printed.
		 *
		 * The print action could either be requested by the user
		 * directly or, for example, by some JS running on the page.
		 *
		 * The hook may cancel the default handler (and no printing
		 * would take place), or, alternatively, it may override the
		 * value of preview by IHookProxy::SetValue() with the name
		 * "preview" and value of type bool.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param browserWidget The browser widget containing the frame
		 * to be printed.
		 * @param preview Whether preview should be done before the
		 * actual printing takes place.
		 * @param frame The frame to be printed.
		 */
		void hookPrint (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				bool preview,
				QWebFrame *frame);

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
		void hookSessionRestoreScheduled (LeechCraft::IHookProxy_ptr proxy,
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
		void hookSetURL (LeechCraft::IHookProxy_ptr proxy,
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
		void hookStatusBarMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString message);

		/** @brief Called from QWebPage::supportsExtension().
		 *
		 * The hook may cancel the default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page whose contents triggered the query for
		 * the given extension support.
		 * @param extension The extension to be queried.
		 *
		 * @sa hookExtension().
		 */
		void hookSupportsExtension (LeechCraft::IHookProxy_ptr proxy,
				const QWebPage *page,
				QWebPage::Extension extension);

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
		void hookTabBarContextMenuActions (LeechCraft::IHookProxy_ptr proxy,
				const QObject *browserWidget) const;

		void hookTabAdded (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QGraphicsWebView *view,
				const QUrl& url);

		void hookTabRemoveRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget);

		/** @brief Called when the given page encounters unsupported
		 * content.
		 *
		 * The hook may choose to handle the reply and cancel the
		 * default implementation.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The page that encountered the unsupported
		 * content.
		 * @param reply The QNetworkReply with the unsupported content.
		 */
		void hookUnsupportedContent (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QNetworkReply *reply);
		void hookUpdateLogicalPath (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookURLCompletionNewStringRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *model,
				const QString& string,
				int historyItems);
		void hookURLEditReturnPressed (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookUserAgentForUrlRequested (LeechCraft::IHookProxy_ptr proxy,
				const QUrl& url,
				const QWebPage* sourcePage);

		/** @brief Called when the given page begins constructing.
		 *
		 * This hook is useful for doing actions that should be done for
		 * each page constructed.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage that begins constructing.
		 */
		void hookWebPageConstructionBegin (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called when the given page finishes constructing.
		 *
		 * This hook is analogous to hookWebPageConstructionBegin(), but
		 * it is called in the end of initialization process.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The QWebPage that finished constructing.
		 *
		 * @sa hookWebPageConstructionBegin().
		 */
		void hookWebPageConstructionEnd (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);

		/** @brief Called from QWebPluginFactory::refreshPlugins().
		 *
		 * This hook may be used to inject plugins derived from
		 * IWebPlugin into the WebKit's QWebPluginFactory. Just append
		 * the required plugins to the plugins list.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param plugins The list of plugins.
		 */
		void hookWebPluginFactoryReload (LeechCraft::IHookProxy_ptr proxy,
				QList<IWebPlugin*>& plugins);

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
		 * @param view The QGraphicsWebView for which the context menu is
		 * requested.
		 * @param event The event object that triggered the context
		 * menu.
		 * @param hitTestResult The result of the
		 * QWebFrame::hitTestContent().
		 * @param menu The menu being built.
		 * @param menuBuildStage The stage of the menu being built.
		 */
		void hookWebViewContextMenu (LeechCraft::IHookProxy_ptr,
				QGraphicsWebView*,
				QGraphicsSceneContextMenuEvent*,
				const QWebHitTestResult&, QMenu*,
				WebViewCtxMenuStage);

		/** @brief Called from QWebPage::windowCloseRequested().
		 *
		 * This signal is called when the window close is requested by
		 * the page.
		 *
		 * @param proxy The standard hook proxy object.
		 * @param page The web page originating the request.
		 */
		void hookWindowCloseRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
	};
}
}

#endif
