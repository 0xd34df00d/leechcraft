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

#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include <stdexcept>
#include <QObject>
#include <qwebpage.h>
#include <qwebframe.h>
#include "iwebplugin.h"

class QNetworkRequest;
class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class IProxyObject;

			/** @brief Base class for all the plugins.
			 *
			 * Provides some kind of interface for communication with
			 * plugins.
			 *
			 * Functions either don't return anything, or return a boolean
			 * value (true means "stop processing", false means "continue"),
			 * or return some custom value. In the later case returning
			 * something means "stop processing and use this value instead
			 * of default", throwing any exception means "continue".
			 */
			class PluginBase
			{
			public:
				/** @brief Initializes the plugin.
				 *
				 * Initializes the plugin with the given proxy object.
				 * Through the proxy object plugin can access and manipulate
				 * LeechCraft::Poshuku's internals.
				 *
				 * @param[in] proxy Pointer to the proxy object.
				 */
				virtual void Init (IProxyObject *proxy) = 0;

				virtual ~PluginBase ()
				{
				}

				/** @brief Reload of the web plugin factory.
				 *
				 * A plugin can insert itself into the list of the
				 * plugins here. It's not recommended to inspect this
				 * list of the plugins as the order of plugins is
				 * not determined.
				 *
				 * @param plugins The list with web plugins.
				 * @return True if the reload and plugin processing
				 * should be stopped, false otherwise.
				 */
				virtual bool HandleWebPluginFactoryReload (QList<IWebPlugin*>& plugins)
				{
					Q_UNUSED (plugins);
					return false;
				}

				/** @brief Begin of web page construction.
				 *
				 * This function is called just in the beginning of
				 * construction of the webpage.
				 *
				 * @param[in] webpage Pointer to the web page being
				 * constructed.
				 * @return True if the web page construction and plugins
				 * processing should be stopped, false otherwise.
				 */
				virtual bool HandleBeginWebPageConstruction (QWebPage *webpage)
				{
					Q_UNUSED (webpage);
					return false;
				}

				/** @brief End of web page construction.
				 *
				 * This function is called just in the end of
				 * construction of the webpage.
				 *
				 * @param[in] webpage Pointer to the web page being
				 * constructed.
				 * @return True if the web page construction and plugins
				 * processing should be stopped, false otherwise.
				 */
				virtual bool HandleEndWebPageConstruction (QWebPage *webpage)
				{
					Q_UNUSED (webpage);
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::contentsChanged() signal.
				 */
				virtual bool HandleContentsChanged (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::databaseQuotaExceeded() signal.
				 */
				virtual bool HandleDatabaseQuotaExceeded (QWebPage*, QWebFrame*, QString)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::downloadRequested() signal.
				 */
				virtual bool HandleDownloadRequested (QWebPage*,
						const QNetworkRequest&)
				{
					return false;
				}

				/** See the official Qt docs for
				 * QWebPage::extension() function.
				 */
				virtual bool HandleExtension (QWebPage*, QWebPage::Extension,
						const QWebPage::ExtensionOption*, QWebPage::ExtensionReturn*)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::frameCreated() signal.
				 */
				virtual bool HandleFrameCreated (QWebPage*, QWebFrame*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::geometryChangeRequested() signal.
				 */
				virtual bool HandleGeometryChangeRequested (QWebPage*, const QRect&)
				{
					return false;
				}

				/** This function is called in a slot connected to
				 * QWebFrame::javaScriptWindowObjectCleared().
				 */
				virtual bool HandleJavaScriptWindowObjectCleared (QWebPage*, QWebFrame*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::linkClicked() signal.
				 */
				virtual bool HandleLinkClicked (QWebPage*, const QUrl&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::linkHovered() signal.
				 */
				virtual bool HandleLinkHovered (QWebPage*,
						const QString&, const QString&, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::loadFinished() signal.
				 */
				virtual bool HandleLoadFinished (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::loadProgress() signal.
				 */
				virtual bool HandleLoadProgress (QWebPage*, int)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::loadStarted() signal.
				 */
				virtual bool HandleLoadStarted (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::menuBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleMenuBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::microFocusChanged() signal.
				 */
				virtual bool HandleMicroFocusChanged (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::printRequested() signal.
				 */
				virtual bool HandlePrintRequested (QWebPage*, QWebFrame*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::repaintRequested() signal.
				 */
				virtual bool HandleRepaintRequested (QWebPage*, const QRect&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::restoreFrameStateRequested() signal.
				 */
				virtual bool HandleRestoreFrameStateRequested (QWebPage*, QWebFrame*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::saveFrameStateRequested() signal.
				 */
				virtual bool HandleSaveFrameStateRequested (QWebPage*, QWebFrame*, QWebHistoryItem*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::scrollRequested() signal.
				 */
				virtual bool HandleScrollRequested (QWebPage*, int, int, const QRect&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::selectionChanged() signal.
				 */
				virtual bool HandleSelectionChanged (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::statusBarMessage() signal.
				 */
				virtual bool HandleStatusBarMessage (QWebPage*, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::statusBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleStatusBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::supportsExtension() function.
				 */
				virtual bool HandleSupportsExtension (const QWebPage*, QWebPage::Extension)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::toolBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleToolBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::unsupportedContent() signal.
				 */
				virtual bool HandleUnsupportedContent (QWebPage*, QNetworkReply*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::windowCloseRequested() signal.
				 */
				virtual bool HandleWindowCloseRequested (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::acceptNavigationRequest().
				 */
				virtual bool OnAcceptNavigationRequest (QWebPage*,
						QWebFrame*,
						const QNetworkRequest&,
						QWebPage::NavigationType)
				{
					return false;
				}

				/** See the official Qt docs for the QWebPage::chooseFile().
				 */
				virtual QString OnChooseFile (QWebPage*, QWebFrame*, const QString&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the QWebPage::createPlugin().
				 */
				virtual QObject* OnCreatePlugin (QWebPage*, const QString&, const QUrl&,
						const QStringList&, const QStringList&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the QWebPage::createWindow().
				 */
				virtual QWebPage* OnCreateWindow (QWebPage*, QWebPage::WebWindowType)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptAlert().
				 */
				virtual bool OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptConfirm().
				 */
				virtual bool OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptConsoleMessage().
				 */
				virtual bool OnJavaScriptConsoleMessage (QWebPage*, const QString&,
						int, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptPrompt().
				 */
				virtual bool OnJavaScriptPrompt (QWebPage*, QWebFrame*, const QString&,
						const QString&, QString*)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::userAgentForUrl().
				 */
				virtual QString OnUserAgentForUrl (const QWebPage*, const QUrl&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** Enumartion describing the part of menu that's being
				 * constructed inside QWebView's subclass'
				 * contextMenuEvent.
				 */
				enum WebViewCtxMenuStage
				{
					/// Just the beginning of menu construction.
					WVSStart,
					/// Stage related to clicking on a hyperlink finished.
					WVSAfterLink,
					/// Stage related to clicking on an image finished.
					WVSAfterImage,
					/// Stage related to clicking with having some
					/// selected text finished.
					WVSAfterSelectedText,
					/// The standard set of actions was embedded, This
					/// stage is just before executing the menu.
					WVSAfterFinish
				};

				/** Called inside QWebView's subclass' context menu
				 * event on different stages with different values
				 * of stage. All stages are passed regardless whether
				 * their ocnditions where met like selected text is
				 * present for WVSAfterSelectedText stage.
				 *
				 * @param[in/out] view The QWebView where all this
				 * happens.
				 * @param[in] e The context menu event.
				 * @param[in] r The result of performing
				 * QWebFrame::hitTestContent(). This one is used
				 * through the whole procedure of building the menu.
				 * @param[in/out] menu The menu that is being constructed.
				 * @param[in] stage Curretn stage.
				 */
				virtual bool OnWebViewCtxMenu (QWebView *view,
						QContextMenuEvent *e,
						const QWebHitTestResult& r,
						QMenu *menu,
						WebViewCtxMenuStage stage)
				{
					Q_UNUSED (view);
					Q_UNUSED (e);
					Q_UNUSED (r);
					Q_UNUSED (menu);
					Q_UNUSED (stage);
					return false;
				}
			};

			typedef PluginBase *PluginBase_ptr;
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Poshuku::PluginBase,
		"org.Deviant.LeechCraft.Plugins.Poshuku.PluginBase/1.0");

#endif

