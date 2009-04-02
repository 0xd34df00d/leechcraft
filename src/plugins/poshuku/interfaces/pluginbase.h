#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include <QObject>
#include <QWebPage>

class QNetworkRequest;

namespace LeechCraft
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
		class PluginBase : public QObject
		{
			Q_OBJECT
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

			/** See the official Qt docs for the
			 * QWebPage::contentsChanged() signal.
			 */
			virtual bool HandleContentsChanged (QWebPage*);

			/** See the official Qt docs for the
			 * QWebPage::databaseQuotaExceeded() signal.
			 */
			virtual bool HandleDatabaseQuotaExceeded (QWebPage*, QWebFrame*, QString);

			/** See the official Qt docs for the
			 * QWebPage::downloadRequested() signal.
			 */
			virtual bool HandleDownloadRequested (QWebPage*,
					const QNetworkRequest&);

			/** See the official Qt docs for the
			 * QWebPage::frameCreated() signal.
			 */
			virtual bool HandleFrameCreated (QWebPage*, QWebFrame*);

			/** See the official Qt docs for the
			 * QWebPage::geometryChangeRequested() signal.
			 */
			virtual bool HandleGeometryChangeRequested (QWebPage*, const QRect&);

			/** See the official Qt docs for the
			 * QWebPage::linkClicked() signal.
			 */
			virtual bool HandleLinkClicked (QWebPage*, const QUrl&);

			/** See the official Qt docs for the
			 * QWebPage::linkHovered() signal.
			 */
			virtual bool HandleLinkHovered (QWebPage*,
					const QString&, const QString&, const QString&);

			/** See the official Qt docs for the
			 * QWebPage::loadFinished() signal.
			 */
			virtual bool HandleLoadFinished (QWebPage*, bool);

			/** See the official Qt docs for the
			 * QWebPage::loadProgress() signal.
			 */
			virtual bool HandleLoadProgress (QWebPage*, int);

			/** See the official Qt docs for the
			 * QWebPage::loadStarted() signal.
			 */
			virtual bool HandleLoadStarted (QWebPage*);

			/** See the official Qt docs for the
			 * QWebPage::menuBarVisibilityChangeRequested() signal.
			 */
			virtual bool HandleMenuBarVisibilityChangeRequested (QWebPage*, bool);

			/** See the official Qt docs for the
			 * QWebPage::microFocusChanged() signal.
			 */
			virtual bool HandleMicroFocusChanged (QWebPage*);

			/** See the official Qt docs for the
			 * QWebPage::printRequested() signal.
			 */
			virtual bool HandlePrintRequested (QWebPage*, QWebFrame*);

			/** See the official Qt docs for the
			 * QWebPage::repaintRequested() signal.
			 */
			virtual bool HandleRepaintRequested (QWebPage*, const QRect&);

			/** See the official Qt docs for the
			 * QWebPage::restoreFrameStateRequested() signal.
			 */
			virtual bool HandleRestoreFrameStateRequested (QWebPage*, QWebFrame*);

			/** See the official Qt docs for the
			 * QWebPage::saveFrameStateRequested() signal.
			 */
			virtual bool HandleSaveFrameStateRequested (QWebPage*, QWebFrame*, QWebHistoryItem*);

			/** See the official Qt docs for the
			 * QWebPage::scrollRequested() signal.
			 */
			virtual bool HandleScrollRequested (QWebPage*, int, int, const QRect&);

			/** See the official Qt docs for the
			 * QWebPage::selectionChanged() signal.
			 */
			virtual bool HandleSelectionChanged (QWebPage*);

			/** See the official Qt docs for the
			 * QWebPage::statusBarMessage() signal.
			 */
			virtual bool HandleStatusBarMessage (QWebPage*, const QString&);

			/** See the official Qt docs for the
			 * QWebPage::statusBarVisibilityChangeRequested() signal.
			 */
			virtual bool HandleStatusBarVisibilityChangeRequested (QWebPage*, bool);

			/** See the official Qt docs for the
			 * QWebPage::toolBarVisibilityChangeRequested() signal.
			 */
			virtual bool HandleToolBarVisibilityChangeRequested (QWebPage*, bool);

			/** See the official Qt docs for the
			 * QWebPage::unsupportedContent() signal.
			 */
			virtual bool HandleUnsupportedContent (QWebPage*, QNetworkReply*);

			/** See the official Qt docs for the
			 * QWebPage::windowCloseRequested() signal.
			 */
			virtual bool HandleWindowCloseRequested (QWebPage*);

			/** See the official Qt docs for the
			 * QWebPage::acceptNavigationRequest().
			 */
			bool OnAcceptNavigationRequest (QWebPage*,
					QWebFrame*,
					const QNetworkRequest&,
					QWebPage::NavigationType);

			/** See the official Qt docs for the QWebPage::chooseFile().
			 */
			QString OnChooseFile (QWebPage*, QWebFrame*, const QString&);

			/** See the official Qt docs for the QWebPage::createPlugin().
			 */
			QObject* OnCreatePlugin (QWebPage*, const QString&, const QUrl&,
					const QStringList&, const QStringList&);

			/** See the official Qt docs for the QWebPage::createWindow().
			 */
			QWebPage* OnCreateWindow (QWebPage*, QWebPage::WebWindowType);

			/** See the official Qt docs for the
			 * QWebPage::javaScriptAlert().
			 */
			bool OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&);

			/** See the official Qt docs for the
			 * QWebPage::javaScriptConfirm().
			 */
			bool OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&);

			/** See the official Qt docs for the
			 * QWebPage::javaScriptConsoleMessage().
			 */
			bool OnJavaScriptConsoleMessage (QWebPage*, const QString&,
					int, const QString&);

			/** See the official Qt docs for the
			 * QWebPage::javaScriptPrompt().
			 */
			bool OnJavaScriptPrompt (QWebPage*, QWebFrame*, const QString&,
					const QString&, QString*);

			/** See the official Qt docs for the
			 * QWebPage::userAgentForUrl().
			 */
			QString OnUserAgentForUrl (const QWebPage*, const QUrl&);
		};

		typedef PluginBase *PluginBase_ptr;
	};
};

#endif

