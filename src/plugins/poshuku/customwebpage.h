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

#ifndef PLUGINS_POSHUKU_PLUGINS_POSHUKU_CUSTOMWEBPAGE_H
#define PLUGINS_POSHUKU_PLUGINS_POSHUKU_CUSTOMWEBPAGE_H
#include <boost/shared_ptr.hpp>
#include <qwebpage.h>
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/iinfo.h>
#include "pageformsdata.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class JSProxy;
			class ExternalProxy;

			class CustomWebPage : public QWebPage
			{
				Q_OBJECT

				Qt::MouseButtons MouseButtons_;
				Qt::KeyboardModifiers Modifiers_;

				QUrl LoadingURL_;
				boost::shared_ptr<JSProxy> JSProxy_;
				boost::shared_ptr<ExternalProxy> ExternalProxy_;
				typedef QMap<QWebFrame*, QWebHistoryItem*> Frame2History_t;
				Frame2History_t Frame2History_;

				QMap<ErrorDomain, QMap<int, QStringList> > Error2Suggestions_;
			public:
				CustomWebPage (QObject* = 0);
				virtual ~CustomWebPage ();

				void SetButtons (Qt::MouseButtons);
				void SetModifiers (Qt::KeyboardModifiers);
				bool supportsExtension (Extension) const;
				bool extension (Extension, const ExtensionOption*, ExtensionReturn*);
			private slots:
				void handleContentsChanged ();
				void handleDatabaseQuotaExceeded (QWebFrame*, QString);
				void handleDownloadRequested (const QNetworkRequest&);
				void handleFrameCreated (QWebFrame*);
				void handleJavaScriptWindowObjectCleared ();
				void handleGeometryChangeRequested (const QRect&);
				void handleLinkClicked (const QUrl&);
				void handleLinkHovered (const QString&, const QString&, const QString&);
				void handleLoadFinished (bool);
				void handleLoadStarted ();
				void handleUnsupportedContent (QNetworkReply*);
				void handleWindowCloseRequested ();
				void fillForms (QWebFrame*);
			protected:
				virtual bool acceptNavigationRequest (QWebFrame*,
						const QNetworkRequest&, QWebPage::NavigationType);
				virtual QString chooseFile (QWebFrame*, const QString&);
				virtual QObject* createPlugin (const QString&, const QUrl&,
						const QStringList&, const QStringList&);
				virtual QWebPage* createWindow (WebWindowType);
				virtual void javaScriptAlert (QWebFrame*, const QString&);
				virtual bool javaScriptConfirm (QWebFrame*, const QString&);
				virtual void javaScriptConsoleMessage (const QString&, int, const QString&);
				virtual bool javaScriptPrompt (QWebFrame*, const QString&, const QString&, QString*);
				virtual QString userAgentForUrl (const QUrl&) const;
			private:
				QString MakeErrorReplyContents (int, const QUrl&,
						const QString&, ErrorDomain = WebKit) const;
				QWebFrame* FindFrame (const QUrl&);
				void HandleForms (QWebFrame*, const QNetworkRequest&,
						QWebPage::NavigationType);
			signals:
				void gotEntity (const LeechCraft::Entity&);
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
				void loadingURL (const QUrl&);
				void storeFormData (const PageFormsData_t&);
				void couldHandle (const LeechCraft::Entity&, bool*);
				void delayedFillForms (QWebFrame*);

				// Hook support signals
				void hookAcceptNavigationRequest (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page,
						QWebFrame *frame,
						QNetworkRequest *request,
						QWebPage::NavigationType type);
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
				void hookFrameCreated (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page,
						QWebFrame *frameCreated);
				void hookGeometryChangeRequested (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page,
						QRect *rect);
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
				void hookLoadStarted (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page);
				void hookSupportsExtension (LeechCraft::IHookProxy_ptr proxy,
						const QWebPage *page,
						QWebPage::Extension extension) const;
				void hookUnsupportedContent (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page,
						QNetworkReply *reply);
				void hookWebPageConstructionFinished (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page);
				void hookWebPageConstructionStarted (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page);
				void hookWindowCloseRequested (LeechCraft::IHookProxy_ptr proxy,
						QWebPage *page);
			};
		};
	};
};

#endif

