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

#ifndef PLUGINS_POSHUKU_PLUGINMANAGER_H
#define PLUGINS_POSHUKU_PLUGINMANAGER_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QWebPage>
#include <interfaces/iinfo.h>
#include "interfaces/poshukutypes.h"
#include "interfaces/iwebplugin.h"

class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ProxyObject;
			class Core;

			class PluginManager : public QObject
			{
				Q_OBJECT

				std::vector<QObject*> Plugins_;
				boost::shared_ptr<ProxyObject> ProxyObject_;
			public:
				PluginManager (QObject* = 0);

				void AddPlugin (QObject*);

				void RegisterHookable (QObject*);
			signals:
				void hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page,
						QWebFrame *frame,
						QNetworkRequest *request,
						QWebPage::NavigationType type);
				void hookAddedToFavorites (LeechCraft::IHookProxy_ptr,
						QString title, QString url, QStringList tags);
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
						const QUrl& url,
						QIcon *icon);
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
		};
	};
};

#endif

