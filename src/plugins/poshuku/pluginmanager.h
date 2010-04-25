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
#include "interfaces/pluginbase.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ProxyObject;

			class PluginManager : public QObject
								, public PluginBase
			{
				Q_OBJECT
				Q_INTERFACES (LeechCraft::Plugins::Poshuku::PluginBase)

				std::vector<PluginBase_ptr> Plugins_;
				boost::shared_ptr<ProxyObject> ProxyObject_;
			public:
				PluginManager (QObject* = 0);

				void AddPlugin (QObject*);

				void Init (IProxyObject*);
				bool HandleWebPluginFactoryReload (QList<IWebPlugin*>&);
				bool HandleBeginWebPageConstruction (QWebPage*);
				bool HandleEndWebPageConstruction (QWebPage*);
				bool HandleContentsChanged (QWebPage*);
				bool HandleDatabaseQuotaExceeded (QWebPage*, QWebFrame*, QString);
				bool HandleDownloadRequested (QWebPage*, const QNetworkRequest&);
				bool HandleExtension (QWebPage*, QWebPage::Extension,
						const QWebPage::ExtensionOption*, QWebPage::ExtensionReturn*);
				bool HandleFrameCreated (QWebPage*, QWebFrame*);
				bool HandleGeometryChangeRequested (QWebPage*, const QRect&);
				bool HandleJavaScriptWindowObjectCleared (QWebPage*, QWebFrame*);
				bool HandleLinkClicked (QWebPage*, const QUrl&);
				bool HandleLinkHovered (QWebPage*, const QString&,
						const QString&, const QString&);
				bool HandleLoadFinished (QWebPage*, bool);
				bool HandleLoadProgress (QWebPage*, int);
				bool HandleLoadStarted (QWebPage*);
				bool HandleMenuBarVisibilityChangeRequested (QWebPage*, bool);
				bool HandleMicroFocusChanged (QWebPage*);
				bool HandlePrintRequested (QWebPage*, QWebFrame*);
				bool HandleRepaintRequested (QWebPage*, const QRect&);
				bool HandleRestoreFrameStateRequested (QWebPage*, QWebFrame*);
				bool HandleSaveFrameStateRequested (QWebPage*, QWebFrame*, QWebHistoryItem*);
				bool HandleScrollRequested (QWebPage*, int, int, const QRect&);
				bool HandleSelectionChanged (QWebPage*);
				bool HandleStatusBarMessage (QWebPage*, const QString&);
				bool HandleStatusBarVisibilityChangeRequested (QWebPage*, bool);
				bool HandleSupportsExtension (const QWebPage*, QWebPage::Extension);
				bool HandleToolBarVisibilityChangeRequested (QWebPage*, bool);
				bool HandleUnsupportedContent (QWebPage*, QNetworkReply*);
				bool HandleWindowCloseRequested (QWebPage*);
				bool OnAcceptNavigationRequest (QWebPage*, QWebFrame*,
						const QNetworkRequest&, QWebPage::NavigationType);
				QString OnChooseFile (QWebPage*, QWebFrame*, const QString&);
				QObject* OnCreatePlugin (QWebPage*, const QString&, const QUrl&,
						const QStringList&, const QStringList&);
				QWebPage* OnCreateWindow (QWebPage*, QWebPage::WebWindowType);
				bool OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&);
				bool OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&);
				bool OnJavaScriptConsoleMessage (QWebPage*, const QString&,
						int, const QString&);
				bool OnJavaScriptPrompt (QWebPage*, QWebFrame*, const QString&,
						const QString&, QString*);
				QString OnUserAgentForUrl (const QWebPage*, const QUrl&);
				bool OnWebViewCtxMenu (QWebView*, QContextMenuEvent*,
						const QWebHitTestResult&, QMenu*,
						WebViewCtxMenuStage);
			};
		};
	};
};

#endif

