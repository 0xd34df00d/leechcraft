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
			 * QWebPage::handleDownloadRequested().
			 */
			virtual bool OnHandleDownloadRequested (QWebPage*,
					const QNetworkRequest&);

			/** See the official Qt docs for the
			 * QWebPage::gotUnsupportedContent().
			 */
			virtual bool OnGotUnsupportedContent (QWebPage*, QNetworkReply*);

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

