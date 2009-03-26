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

			/** @brief Called in the beginning of handling of download
			 * request.
			 *
			 * This function is called in the beginning of handling
			 * user's request for a download of an object.
			 * The default implementation does nothing and returns
			 * false.
			 *
			 * @param[in] page Pointer to the QWebPage that generated
			 * this event.
			 * @param[in] request The QNetworkRequest in question.
			 * @return Whether the processing should be stopped.
			 */
			virtual bool OnHandleDownloadRequested (QWebPage *page,
					const QNetworkRequest& request);

			/** @brief Called in the beginning of handling of
			 * unsupported content.
			 *
			 * This function is called when the browser recieves the
			 * content it can't handle.
			 *
			 * The default implementation does nothing and returns
			 * false.
			 *
			 * @param[in] page Pointer to the QWebPage that generated
			 * this event.
			 * @param[in] reply The QNetworkReply associated with the
			 * unsupported content..
			 * @return Whether the processing should be stopped.
			 */
			virtual bool OnGotUnsupportedContent (QWebPage *page,
					QNetworkReply *reply);

			/** @brief Called when a navigation request should be either
			 * accepter or rejected.
			 *
			 * If this function returns true, the request is immidiately
			 * rejected, otherwise it is further processed with other
			 * plugins and then via Poshuku core.
			 *
			 * @param[in] page Pointer to the QWebPage that generated
			 * this event.
			 * @param[in] frame Pointer to the QWebFrame that issued the
			 * request.
			 * @param[in] request Request in question.
			 * @param[in] type The navigation type.
			 * @return Whether the request should be rejected or
			 * processed further.
			 */
			bool OnAcceptNavigationRequest (QWebPage *page,
					QWebFrame *frame,
					const QNetworkRequest& request,
					QWebPage::NavigationType type);

			/** @brief Called when something requests user agent.
			 *
			 * Returns the user agent or throws anything by default.
			 *
			 * @param[in] page Pointer to the QWebPage that generated
			 * this event.
			 * @param[in] url URL that requests the user agent.
			 * @return User agent for the URL.
			 */
			QString OnUserAgentForUrl (const QWebPage *page, const QUrl& url);
		};

		typedef PluginBase *PluginBase_ptr;
	};
};

#endif

