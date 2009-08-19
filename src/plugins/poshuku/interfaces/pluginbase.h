#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include <stdexcept>
#include <QObject>
#include <QWebPage>

class QNetworkRequest;

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
			};

			typedef PluginBase *PluginBase_ptr;
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Poshuku::PluginBase,
		"org.Deviant.LeechCraft.Plugins.Poshuku.PluginBase/1.0");

#endif

