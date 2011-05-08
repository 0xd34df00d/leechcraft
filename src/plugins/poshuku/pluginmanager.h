/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <boost/shared_ptr.hpp>
#include <QWebPage>
#include <QDateTime>
#include <plugininterface/basehookinterconnector.h>
#include <interfaces/iinfo.h>
#include "interfaces/poshukutypes.h"
#include "interfaces/iwebplugin.h"

class QWebView;

namespace LeechCraft
{
namespace Poshuku
{
	class ProxyObject;
	class Core;

	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT

		boost::shared_ptr<ProxyObject> ProxyObject_;
	public:
		PluginManager (QObject* = 0);

		virtual void AddPlugin (QObject*);
	signals:
		/** @brief Called whenever WebKit requests to navigate frame to
		 * the resource specified by request.
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
		 * @param proxy The standard hook proxy class.
		 * @param page The QWebPage from which this request originates.
		 * @param frame The frame to navigate, or null pointer if
		 * navigating to new window is requested.
		 * @param request The original network request.
		 * @param type The navigation type.
		 */
		void hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QNetworkRequest *request,
				QWebPage::NavigationType type);
		
		/** @brief Called when an entry is just added to the favorites.
		 * 
		 * This hook is emitted just after an entry with given title,
		 * url and tags is added to favorites. At this point, canceling
		 * the default handler or trying to change other parameters
		 * doesn't make sense. Refer to hookAddToFavoritesRequested()
		 * if you need to modify these.
		 * 
		 * @param proxy The standard hook proxy class.
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
		 * If the proxy is cancelled, no addition takes place at all,
		 * otherwise, the return value from the proxy is considered to
		 * be a list of QVariants. First element (if any) is converted
		 * to string and overrides the history entry title, second one
		 * (if any) is converted to string as well and overrides url,
		 * and third element (if any) is converted to QDateTime and
		 * overrides the date.
		 *
		 * @param proxy The standard hook proxy class.
		 * @param title The title of the item that's going to be added
		 * to history.
		 * @param url The URL of the item.
		 * @param date Datetime of visit (usually current one).
		 * @param browserWidget The BrowserWidget from which this
		 * request originates.
		 */
		void hookAddingToHistory (LeechCraft::IHookProxy_ptr proxy,
				QString title, QString url, QDateTime date, QObject *browserWidget);

		void hookAddToFavoritesRequested (LeechCraft::IHookProxy_ptr,
				QString *title, QString *url);
		void hookChooseFile (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString *suggested);
		void hookContentsChanged (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
		void hookCreatePlugin (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString *clsid,
				QUrl *url,
				QStringList *params,
				QStringList *values);
		void hookCreateWindow (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::WebWindowType type);
		void hookDatabaseQuotaExceeded (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *sourceFrame,
				QString databaseName);
		void hookDownloadRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QNetworkRequest *downloadRequest);
		void hookExtension (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebPage::Extension extension,
				const QWebPage::ExtensionOption* extensionOption,
				QWebPage::ExtensionReturn* extensionReturn);
		void hookFindText (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString *findText,
				QWebPage::FindFlags *findFlags);
		void hookFrameCreated (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frameCreated);
		void hookGeometryChangeRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QRect *rect);
		void hookIconChanged (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget);
		void hookIconRequested (LeechCraft::IHookProxy_ptr,
				const QUrl& url);
		void hookInitialLayoutCompleted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame);
		void hookJavaScriptAlert (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString *msg);
		void hookJavaScriptConfirm (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString *msg);
		void hookJavaScriptConsoleMessage (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString *msg,
				int *line,
				QString *sourceId);
		void hookJavaScriptPrompt (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QWebFrame *frame,
				QString *msg,
				QString *defValue,
				QString *resultString);
		void hookJavaScriptWindowObjectCleared (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *sourcePage,
				QWebFrame *frameCleared);
		void hookLinkClicked (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QUrl *url);
		void hookLinkHovered (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				QString *link,
				QString *title,
				QString *textContent);
		void hookLoadFinished (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page,
				bool *result);
		void hookLoadProgress (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				int *progress);
		void hookLoadStarted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
		void hookMoreMenuFillBegin (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu,
				QWebView *webView,
				QObject *browserWidget);
		void hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu,
				QWebView *webView,
				QObject *browserWidget);
		void hookNotificationActionTriggered (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				int *index);
		void hookNotifyLoadFinished (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				bool *ok,
				bool notifyWhenFinished,
				bool own,
				bool htmlMode);
		void hookPrint (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				bool *preview,
				QWebFrame *frame);
		void hookSetURL (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QUrl *url);
		void hookStatusBarMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *browserWidget,
				QString *message);
		void hookSupportsExtension (LeechCraft::IHookProxy_ptr proxy,
				const QWebPage *page,
				QWebPage::Extension extension);
		void hookTabBarContextMenuActions (LeechCraft::IHookProxy_ptr proxy,
				const QObject *browserWidget,
				QList<QAction*>*) const;
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
		void hookWebPageConstructionFinished (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
		void hookWebPageConstructionStarted (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
		void hookWebPluginFactoryReload (LeechCraft::IHookProxy_ptr proxy,
				QList<IWebPlugin*>& plugins);
		void hookWebViewContextMenu (LeechCraft::IHookProxy_ptr proxy,
				QWebView *sourceView,
				QContextMenuEvent *event,
				const QWebHitTestResult& hitTestResult,
				QMenu *menuBeingBuilt,
				WebViewCtxMenuStage menuBuildStage);
		void hookWindowCloseRequested (LeechCraft::IHookProxy_ptr proxy,
				QWebPage *page);
	};
}
}

#endif
